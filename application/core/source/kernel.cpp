#include "kernel.h"

#include <cassert>
#include <format>

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

bool Kernel::Dispatch(Request &request, const SpectrumAllocator &allocator) {
  assert(request.FSUs <= spectrum.size());

  const auto keys = configuration->keyGenerator.generate(request.route);

  const auto first = *keys.begin();

  const auto slice = allocator(carriers[first], request.FSUs);

  if (!slice.has_value()) {
    return false;
  }

  request.slice = slice.value();

  for (const auto &key : keys) {
    if (carriers.at(key).available() < request.FSUs ||
        !carriers.at(key).available_at(request.slice)) {
      return false;
    }
  }

  const auto allocate = [&](const auto key) {
    carriers[key].allocate(request.slice);
  };

  std::for_each(keys.begin(), keys.end(), allocate);

  return true;
}

void Kernel::Release(Request &request) {
  const auto keys = configuration->keyGenerator.generate(request.route);

  std::for_each(keys.begin(), keys.end(), [&](const auto key) {
    assert(!carriers.at(key).available_at(slice));

    carriers[key].deallocate(request.slice);
  });
}

void Kernel::ScheduleNextArrival(void) {
  auto &request = configuration->requests[requestsKeys[prng->next("fsus")]];

  ++request.counting;

  queue.push(Event(
      time + prng->next("arrival"), EventType::Arrival,
      Request(router.compute(NullVertex, NullVertex).value(), request.FSUs)));

  ++requestCount;
}

void Kernel::ScheduleNextDeparture(const Event &event) {
  queue.push(
      Event(time + prng->next("service"), EventType::Departure, event.request));
}

Kernel::Kernel(std::shared_ptr<Configuration> configuration)
    : k_to_ignore{0.1 * configuration->timeUnits},
      configuration{configuration} {
  for (const auto &[source, destination, cost] :
       configuration->graph.get_edges()) {
    const auto key = configuration->keyGenerator.generate(source, destination);

    carriers[key] = Spectrum(configuration->FSUsPerLink);
  }

  Reset();
}

bool Kernel::HasNext(void) const {
  return !queue.empty() && queue.top().time <= configuration->timeUnits;
}

void Kernel::Next(void) {
  auto event = queue.top();

  queue.pop();

  time = event.time;

  if (configuration->ignoreFirst && time > k_to_ignore && !ignored_first_k) {
    ignored_first_k = true;

    requestCount = 0u;

    blockedCount = 0u;

    for (auto &request : configuration->requests) {
      request.second.blocking = 0u;

      request.second.counting = 0u;
    }

    configuration->logger->log(Logger::Level::Info,
                               "Discard first {:.3f} time units", time);
  }

  if (event.type == EventType::Departure) {
    --activeRequests;

    configuration->logger->log(Logger::Level::Info,
                               "Request for {} FSU(s) departing at {:.3f}",
                               event.request.FSUs, event.time);

    Release(event.request);

    return;
  }

  SpectrumAllocator allocator;

  for (auto &requestType : configuration->requests) {
    const ModulationStrategyFactory factory;

    const auto spectralEfficiency =
        configuration->modulations.at(requestType.second.modulation);

    const auto strategy =
        factory.From(configuration->modulationOption, configuration->slotWidth,
                     spectralEfficiency);

    const auto FSUs = configuration->modulationOption ==
                              ModulationStrategyFactory::Option::Passband
                          ? strategy->compute(requestType.second.bandwidth)
                          : strategy->compute(event.request.route.second.value);

    if (FSUs == event.request.FSUs) {
      allocator =
          *requestType.second.allocator
               .target<std::optional<Slice> (*)(const Spectrum &, uint64_t)>();

      break;
    }
  }

  event.request.accepted = false;

  if (activeRequests < configuration->FSUsPerLink &&
      Dispatch(event.request, allocator)) {
    ++activeRequests;

    configuration->logger->log(Logger::Level::Info,
                               "Accept request for {} FSU(s) at {:.3f}",
                               event.request.FSUs, time);

    event.request.accepted = true;

    ScheduleNextDeparture(event);
  } else {
    configuration->logger->log(Logger::Level::Info,
                               "Blocking request for {} FSU(s) at {:.3f}",
                               event.request.FSUs, event.time);

    for (auto &request : configuration->requests) {
      if (event.request.FSUs != request.second.FSUs) {
        continue;
      }

      ++request.second.blocking;

      break;
    }

    ++blockedCount;
  }

  if (snapshots.empty() ||
      abs(snapshots.back().time - event.time) >= configuration->samplingTime) {
    snapshots.push_back(
        Snapshot(event, GetFragmentation(), GetGradeOfService()));
  }

  ScheduleNextArrival();
}

std::shared_ptr<PseudoRandomNumberGenerator>
Kernel::GetPseudoRandomNumberGenerator(void) const {
  return prng;
}

std::vector<Snapshot> Kernel::GetSnapshots(void) const { return snapshots; }

double Kernel::GetTime(void) const { return time; }

double Kernel::GetRequestCount(void) const {
  return static_cast<double>(requestCount);
}

double Kernel::GetGradeOfService(void) const {
  return static_cast<double>(blockedCount) / static_cast<double>(requestCount);
}

std::vector<double> Kernel::GetFragmentation(void) const {
  std::vector<double> fragmentation;

  for (const auto &[_, strategy] : configuration->fragmentationStrategies) {
    double sum = 0.0;

    for (const auto &[source, destination, cost] :
         configuration->graph.get_edges()) {
      const auto key =
          configuration->keyGenerator.generate(source, destination);

      sum += (*strategy)(carriers.at(key));
    }

    fragmentation.push_back(sum);
  }

  return fragmentation;
}

void Kernel::Reset(void) {
  ignored_first_k = false;

  requestCount = 0u;

  blockedCount = 0u;

  activeRequests = 0u;

  time = .0f;

  snapshots.clear();

  requestsKeys.clear();

  std::transform(configuration->requests.begin(), configuration->requests.end(),
                 std::back_inserter(requestsKeys), [](auto &pair) {
                   pair.second.blocking = 0;

                   pair.second.counting = 0;

                   return pair.first;
                 });

  prng = PseudoRandomNumberGenerator::Instance();

  prng->random_seed();

  prng->exponential("arrival", configuration->arrivalRate);

  prng->exponential("service", configuration->serviceRate);

  prng->discrete("fsus", configuration->probs.begin(),
                 configuration->probs.end());

  prng->uniform("routing", 0, configuration->graph.size());

  router.SetStrategy(std::make_shared<RandomRouting>(configuration->graph));

  ScheduleNextArrival();
}
