#include "simulation.h"

#include <cassert>
#include <format>
#include <vector>

#include "graph.h"
#include "prng.h"
#include "request.h"

Event::Event(const double time, const EventType &type, const Request &request)
    : time{time}, type{type}, request{request} {}

bool Event::operator<(const Event &other) const noexcept {
  return time > other.time;
}

Snapshot::Snapshot(const Event &event, std::vector<double> fragmentation,
                   double blocking)
    : time{event.time},
      FSUs{event.request.FSUs},
      accepted{event.request.accepted},
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

Simulation::Simulation(Settings &settings,
                       std::shared_ptr<PseudoRandomNumberGenerator> prng)
    : settings{settings},
      _logger{settings.enableLogging},
      kToIgnore{0.1 * settings.timeUnits},
      dispatcher{settings.graph, settings.keyGenerator, settings.FSUsPerLink},
      prng{prng},
      requestsKeys{} {
  requestsKeys.reserve(settings.requests.size());

  std::transform(settings.requests.begin(), settings.requests.end(),
                 std::back_inserter(requestsKeys),
                 [](const auto &pair) { return pair.first; });
  auto &firstRequest = settings.requests[requestsKeys[prng->next("fsus")]];

  ++firstRequest.counting;

  router.SetStrategy(std::make_shared<RandomRouting>(settings.graph));

  queue.push(Event(
      time + prng->next("arrival"), EventType::Arrival,
      Request(router.compute(NullVertex, NullVertex), firstRequest.FSUs)));

  ++requestCount;
}

bool Simulation::HasNext(void) const {
  return !queue.empty() && queue.top().time <= settings.timeUnits;
}

void Simulation::Next(void) {
  auto event = queue.top();

  queue.pop();

  time = event.time;

  if (settings.ignoreFirst && time > kToIgnore && !ignoredFirst) {
    ignoredFirst = true;

    requestCount = 0u;

    blockedCount = 0u;

    for (auto &request : settings.requests) {
      request.second.blocking = 0u;

      request.second.counting = 0u;
    }

    _logger.log(Logger::Level::Info, "Discard first {:.3f} time units", time);
  }

  if (event.type == EventType::Departure) {
    --activeRequests;

    _logger.log(Logger::Level::Info,
                "Request for {} FSU(s) departing at {:.3f}", event.request.FSUs,
                event.time);

    dispatcher.release(event.request);

    for (const auto &key :
         settings.keyGenerator.generate(event.request.route)) {
      _logger.log(Logger::Level::Info,
                  dispatcher.GetCarriers().at(key).Serialize());
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
                          : strategy->compute(event.request.route.cost.value);

    if (FSUs == event.request.FSUs) {
      allocator = *requestType.second.allocator.target<std::optional<Slice> (*)(
          const Spectrum &, unsigned int)>();

      break;
    }
  }

  event.request.accepted = false;

  if (activeRequests < settings.FSUsPerLink &&
      dispatcher.dispatch(event.request, allocator)) {
    ++activeRequests;

    queue.push(Event(time + prng->next("service"), EventType::Departure,
                     event.request));

    _logger.log(Logger::Level::Info, "Accept request for {} FSU(s) at {:.3f}",
                event.request.FSUs, time);

    for (const auto &key :
         settings.keyGenerator.generate(event.request.route)) {
      _logger.log(Logger::Level::Info,
                  dispatcher.GetCarriers().at(key).Serialize());
    }

    event.request.accepted = true;
  } else {
    _logger.log(Logger::Level::Info, "Blocking request for {} FSU(s) at {:.3f}",
                event.request.FSUs, event.time);

    for (auto &request : settings.requests) {
      if (event.request.FSUs != request.second.FSUs) {
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

  auto &request = settings.requests[requestsKeys[prng->next("fsus")]];

  ++request.counting;

  queue.push(
      Event(time + prng->next("arrival"), EventType::Arrival,
            Request(router.compute(NullVertex, NullVertex), request.FSUs)));

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

  auto &firstRequest = settings.requests[requestsKeys[prng->next("fsus")]];

  ++firstRequest.counting;

  router.SetStrategy(std::make_shared<RandomRouting>(settings.graph));

  queue.push(Event(
      time + prng->next("arrival"), EventType::Arrival,
      Request(router.compute(NullVertex, NullVertex), firstRequest.FSUs)));

  ++requestCount;
}
