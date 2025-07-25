#include "simulation.h"

#include <cassert>
#include <format>
#include <vector>

#include "distribution.h"
#include "graph.h"
#include "logger.h"
#include "request.h"

Event::Event(const double time, const EventType &signal, const Request &value)
    : time{time}, signal{signal}, value{value} {}

bool Event::operator<(const Event &other) const noexcept {
  return time > other.time;
}

EventQueue &EventQueue::push(const Request &value, const double now) {
  assert(!pushing);

  pushing = true;

  to_push = value;

  time = now;

  return *this;
}

void EventQueue::of_type(const EventType &signal) {
  assert(pushing);

  auto now = time;

  switch (signal) {
    case EventType::Arrival:
      now += PseudoRandomNumberGenerator::Instance()->next("arrival");
      break;
    case EventType::Departure:
      now += PseudoRandomNumberGenerator::Instance()->next("service");
      break;
  }

  queue.push(Event(now, signal, to_push));

  pushing = false;
}

std::optional<Event> EventQueue::top(void) const {
  if (empty()) {
    return std::nullopt;
  }

  return queue.top();
}

std::optional<Event> EventQueue::pop(void) {
  if (empty()) {
    return std::nullopt;
  }

  const auto event{queue.top()};

  queue.pop();

  return event;
}

bool EventQueue::empty(void) const { return queue.empty(); }

size_t EventQueue::size(void) const { return queue.size(); }

Snapshot::Snapshot(const Event &event, std::vector<double> fragmentation,
                   double blocking)
    : time{event.time},
      FSUs{event.value.FSUs},
      accepted{event.value.accepted},
      fragmentation{fragmentation},
      blocking{blocking} {}

std::string Snapshot::Serialize(void) const {
  static const std::unordered_map<bool, std::string> map = {
      {false, "False"},
      {true, "True"},
  };

  std::string buffer = std::format("{},{},{},", time, FSUs, map.at(accepted));

  for (const auto &value : fragmentation) {
    buffer.append(std::format("{},", value));
  }

  buffer.append(std::format("{}", blocking));

  return buffer;
}

Simulation::Simulation(Settings &settings)
    : settings{settings},
      kToIgnore{0.1 * settings.timeUnits},
      dispatcher{settings.graph, settings.keyGenerator, settings.FSUsPerLink},
      requestsKeys{} {
  requestsKeys.reserve(settings.requests.size());

  std::transform(settings.requests.begin(), settings.requests.end(),
                 std::back_inserter(requestsKeys),
                 [](const auto &pair) { return pair.first; });
  auto &firstRequest =
      settings.requests
          [requestsKeys[PseudoRandomNumberGenerator::Instance()->next("fsus")]];

  ++firstRequest.counting;

  router.SetStrategy(std::make_shared<RandomRouting>(settings.graph));

  queue
      .push(Request{router.compute(NullVertex, NullVertex), firstRequest.FSUs},
            time)
      .of_type(EventType::Arrival);

  ++requestCount;
}

bool Simulation::HasNext(void) const {
  return queue.top().value().time <= settings.timeUnits;
}

void Simulation::Next(void) {
  auto event = queue.pop().value();

  time = event.time;

  if (settings.ignoreFirst && time > kToIgnore && !ignoredFirst) {
    ignoredFirst = true;

    requestCount = 0u;

    blockedCount = 0u;

    for (auto &request : settings.requests) {
      request.second.blocking = 0u;

      request.second.counting = 0u;
    }

    INFO(std::format("Discard first {:.3f} time units", time));
  }

  if (event.signal == EventType::Departure) {
    --activeRequests;

    INFO(std::format("Request for {} FSU(s) departing at {:.3f}",
                     event.value.FSUs, event.time));

    dispatcher.release(event.value);

    for (const auto &key : settings.keyGenerator.generate(event.value.route)) {
      INFO(dispatcher.GetCarriers().at(key).Serialize());
    }

    return;
  }

  SpectrumAllocator allocator;

  for (auto &requestType : settings.requests) {
    const ModulationStrategyFactory factory(settings.modulationOption);

    const auto spectralEfficiency =
        settings.modulations.at(requestType.second.modulation);

    const auto strategy = factory.From(settings.slotWidth, spectralEfficiency);

    const auto FSUs = settings.modulationOption == ModulationOption::Passband
                          ? strategy->compute(requestType.second.bandwidth)
                          : strategy->compute(event.value.route.cost.value);

    if (FSUs == event.value.FSUs) {
      allocator = *requestType.second.allocator.target<std::optional<Slice> (*)(
          const Spectrum &, unsigned int)>();

      break;
    }
  }

  event.value.accepted = false;

  if (activeRequests < settings.FSUsPerLink &&
      dispatcher.dispatch(event.value, allocator)) {
    ++activeRequests;

    queue.push(event.value, time).of_type(EventType::Departure);

    INFO(std::format("Accept request for {} FSU(s) at {:.3f}", event.value.FSUs,
                     time));

    for (const auto &key : settings.keyGenerator.generate(event.value.route)) {
      INFO(dispatcher.GetCarriers().at(key).Serialize());
    }

    event.value.accepted = true;
  } else {
    INFO(std::format("Blocking request for {} FSU(s) at {:.3f}",
                     event.value.FSUs, event.time));

    for (auto &request : settings.requests) {
      if (event.value.FSUs != request.second.FSUs) {
        continue;
      }

      ++request.second.blocking;

      break;
    }

    ++blockedCount;
  }

  if (snapshots.empty() ||
      abs(snapshots.back().time - event.time) >= settings.samplingTime) {
    snapshots.push_back(
        Snapshot(event, GetFragmentation(), GetGradeOfService()));
  }

  auto &request =
      settings.requests
          [requestsKeys[PseudoRandomNumberGenerator::Instance()->next("fsus")]];

  ++request.counting;

  queue
      .push(Request{router.compute(NullVertex, NullVertex), request.FSUs},
            event.time)
      .of_type(EventType::Arrival);

  ++requestCount;
}

std::vector<Snapshot> Simulation::GetSnapshots(void) const { return snapshots; }

double Simulation::GetTime(void) const { return time; }

double Simulation::GetRequestCount(void) const {
  return static_cast<double>(requestCount);
}

double Simulation::GetGradeOfService(void) const {
  return static_cast<double>(blockedCount) / static_cast<double>(requestCount);
}

std::vector<double> Simulation::GetFragmentation(void) const {
  std::vector<double> fragmentation;

  const auto carriers = dispatcher.GetCarriers();

  for (const auto &[_, strategy] : settings.fragmentationStrategies) {
    double sum = 0.0;

    for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
      const auto key = settings.keyGenerator.generate(source, destination);

      sum += (*strategy)(carriers.at(key));
    }

    fragmentation.push_back(sum);
  }

  return fragmentation;
}

void Simulation::Reset(void) {
  ignoredFirst = false;

  requestCount = 0u;

  blockedCount = 0u;

  activeRequests = 0u;

  time = 0.0;

  snapshots.clear();

  requestsKeys.reserve(settings.requests.size());

  std::transform(settings.requests.begin(), settings.requests.end(),
                 std::back_inserter(requestsKeys),
                 [](const auto &pair) { return pair.first; });

  auto &firstRequest =
      settings.requests
          [requestsKeys[PseudoRandomNumberGenerator::Instance()->next("fsus")]];

  ++firstRequest.counting;

  router.SetStrategy(std::make_shared<RandomRouting>(settings.graph));

  queue
      .push(Request{router.compute(NullVertex, NullVertex), firstRequest.FSUs},
            time)
      .of_type(EventType::Arrival);

  ++requestCount;
}
