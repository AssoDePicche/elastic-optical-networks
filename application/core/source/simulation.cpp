#include "simulation.h"

#include <format>
#include <vector>

#include "distribution.h"
#include "event_queue.h"
#include "graph.h"
#include "logger.h"
#include "request.h"

Snapshot::Snapshot(const Event<Request> &event,
                   std::vector<double> fragmentation, double entropy,
                   double blocking)
    : time{event.time},
      FSUs{event.value.FSUs},
      accepted{event.value.accepted},
      fragmentation{fragmentation},
      entropy{entropy},
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

  buffer.append(std::format("{},{}", entropy, blocking));

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
      .of_type(Signal::ARRIVAL);

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

  if (event.signal == Signal::DEPARTURE) {
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

    queue.push(event.value, time).of_type(Signal::DEPARTURE);

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

  snapshots.push_back(
      Snapshot(event, GetFragmentation(), GetEntropy(), GetGradeOfService()));

  auto &request =
      settings.requests
          [requestsKeys[PseudoRandomNumberGenerator::Instance()->next("fsus")]];

  ++request.counting;

  queue
      .push(Request{router.compute(NullVertex, NullVertex), request.FSUs},
            event.time)
      .of_type(Signal::ARRIVAL);

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

double Simulation::GetEntropy(void) const {
  auto metric =
      settings.fragmentationStrategies.at("entropy_based_fragmentation");

  auto sum = 0.0;

  for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
    const auto key = settings.keyGenerator.generate(source, destination);

    sum += (*metric)(dispatcher.GetCarriers().at(key));
  }

  return sum;
}

std::vector<double> Simulation::GetFragmentation(void) const {
  std::vector<double> fragmentation;

  fragmentation.reserve(settings.graph.get_edges().size());

  for (const auto &strategy : settings.fragmentationStrategies) {
    auto sum = 0.0;

    for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
      const auto key = settings.keyGenerator.generate(source, destination);

      sum += (*strategy.second)(dispatcher.GetCarriers().at(key));
    }

    fragmentation.emplace_back(sum / settings.graph.get_edges().size());
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
      .of_type(Signal::ARRIVAL);

  ++requestCount;
}
