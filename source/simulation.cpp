#include "simulation.h"

#include <format>
#include <vector>

#include "distribution.h"
#include "event_queue.h"
#include "graph.h"
#include "logger.h"
#include "request.h"

Snapshot::Snapshot(double time, int slots, bool accepted, double fragmentation,
                   double entropy, double blocking)
    : time{time},
      slots{slots},
      accepted{accepted},
      fragmentation{fragmentation},
      entropy{entropy},
      blocking{blocking} {}

std::string Snapshot::Serialize(void) const {
  const std::unordered_map<bool, std::string> map = {
      {false, "False"},
      {true, "True"},
  };

  return std::format("{},{},{},{},{},{}", time, slots, map.at(accepted),
                     fragmentation, entropy, blocking);
}

Simulation::Simulation(Settings &settings)
    : settings{settings},
      queue{settings.arrivalRate, settings.serviceRate, settings.seed},
      distribution{settings.seed, settings.probs},
      kToIgnore{0.1 * settings.timeUnits},
      hashmap{make_hashmap(settings.graph, settings.bandwidth)},
      requestsKeys{} {
  requestsKeys.reserve(settings.requests.size());

  std::transform(settings.requests.begin(), settings.requests.end(),
                 std::back_inserter(requestsKeys),
                 [](const auto &pair) { return pair.first; });

  auto &firstRequest = settings.requests[requestsKeys[distribution.next()]];

  ++firstRequest.counting;

  queue.push(Request{random_path(settings.graph), firstRequest.resources}, time)
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
                     event.value.bandwidth, event.time));

    const auto keys{route_keys(event.value.route)};

    for (const auto &key : keys) {
      hashmap[key].deallocate(event.value.slice);

      INFO(hashmap[key].to_string());
    }

    return;
  }

  SpectrumAllocator allocator;

  for (auto &requestType : settings.requests) {
    const auto fsus =
        from_modulation(requestType.second.bandwidth,
                        settings.modulations.at(requestType.second.modulation),
                        settings.slotWidth);

    if (fsus == event.value.bandwidth) {
      allocator = *requestType.second.allocator.target<std::optional<Slice> (*)(
          const Spectrum &, unsigned int)>();

      break;
    }
  }

  auto accepted = false;

  if (activeRequests < settings.bandwidth &&
      dispatch_request(event.value, hashmap, allocator)) {
    ++activeRequests;

    queue.push(event.value, time).of_type(Signal::DEPARTURE);

    INFO(std::format("Accept request for {} FSU(s) at {:.3f}",
                     event.value.bandwidth, time));

    const auto keys{route_keys(event.value.route)};

    for (const auto &key : keys) {
      INFO(hashmap[key].to_string());
    }

    accepted = true;
  } else {
    INFO(std::format("Blocking request for {} FSU(s) at {:.3f}",
                     event.value.bandwidth, event.time));

    for (auto &request : settings.requests) {
      if (event.value.bandwidth != request.second.resources) {
        continue;
      }

      ++request.second.blocking;

      break;
    }

    ++blockedCount;
  }

  snapshots.push_back(Snapshot(event.time, event.value.bandwidth, accepted,
                               network_fragmentation(), entropy(),
                               GetGradeOfService()));

  auto &request = settings.requests[requestsKeys[distribution.next()]];

  ++request.counting;

  queue
      .push(Request{random_path(settings.graph), request.resources}, event.time)
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

double Simulation::network_fragmentation(void) {
  auto fragmentation{0.0};

  for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    fragmentation += hashmap[key].fragmentation();
  }

  return (fragmentation / settings.graph.get_edges().size());
}

double Simulation::entropy(void) {
  auto free_slots = 0.0;

  auto occupied_slots = 0.0;

  for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    const auto spectrum = hashmap[key];

    free_slots += spectrum.available();

    occupied_slots += spectrum.size() - spectrum.available();
  }

  if (free_slots == 0.0 || occupied_slots == 0.0) {
    return 0.0;
  }

  free_slots /= settings.bandwidth;

  occupied_slots /= settings.bandwidth;

  return -(occupied_slots * std::log2(occupied_slots) +
           free_slots * std::log2(free_slots));
}
