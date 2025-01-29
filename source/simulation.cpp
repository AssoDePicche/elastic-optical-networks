#include "simulation.h"

#include "event_queue.h"
#include "graph.h"
#include "group.h"
#include "logger.h"
#include "request.h"
#include "statistics.h"
#include "timer.h"

Snapshot::Snapshot(float time, int bandwidth, bool accepted,
                   float fragmentation, float entropy, float blocking)
    : time{time},
      bandwidth{bandwidth},
      accepted{accepted},
      fragmentation{fragmentation},
      entropy{entropy},
      blocking{blocking} {}

auto Snapshot::str(void) const -> std::string {
  return std::to_string(time) + ',' + std::to_string(bandwidth) + ',' +
         (accepted ? "True" : "False") + ',' + std::to_string(fragmentation) +
         ',' + std::to_string(entropy) + ',' + std::to_string(blocking);
}

Simulation::Simulation(Settings& settings)
    : settings{settings},
      group{settings.seed, {50.0, 50.0f}, {3, 5}},
      queue{settings.arrival_rate, settings.service_rate, settings.seed},
      hashmap{make_hashmap(settings.graph, settings.bandwidth)} {
  queue.push(Request{random_path(settings.graph), group.next()}, time)
      .of_type(Signal::ARRIVAL);
}

void Simulation::next(void) {
  const auto top{queue.pop()};

  auto [now, signal, request] = top.value();

  time = now;

  INFO("Now: " + std::to_string(now));

  if (signal == Signal::DEPARTURE) {
    --active_requests;

    INFO("Request for " + std::to_string(request.bandwidth) +
         " bandwidth slots departing");

    const auto keys{route_keys(request.route)};

    for (const auto& key : keys) {
      hashmap[key].deallocate(request.slice);

      INFO(hashmap[key].to_string());

      INFO("Free (u): " + std::to_string(hashmap[key].available()) +
           " Fragmentation (%): " +
           std::to_string(hashmap[key].fragmentation()));
    }

    return;
  }

  const auto allocator = (request.bandwidth == 3) ? first_fit : last_fit;

  auto accepted = false;

  if (active_requests < settings.bandwidth &&
      dispatch_request(request, hashmap, allocator)) {
    ++active_requests;

    ++accepted_requests;

    queue.push(request, now).of_type(Signal::DEPARTURE);

    INFO("Accept request for " + std::to_string(request.bandwidth) +
         " bandwidth slots");

    const auto keys{route_keys(request.route)};

    for (const auto& key : keys) {
      INFO(hashmap[key].to_string());

      INFO("Free (u): " + std::to_string(hashmap[key].available()) +
           " Fragmentation (%): " +
           std::to_string(hashmap[key].fragmentation()));
    }

    accepted = true;

  } else {
    ++blocked_requests;

    INFO("Blocking request for " + std::to_string(request.bandwidth) +
         " bandwidth slots");

    group.count_blocking(request.bandwidth);
  }

  snapshots.push_back(Snapshot(now, request.bandwidth, accepted,
                               fragmentation(), entropy(), blocking()));

  queue.push(Request{random_path(settings.graph), group.next()}, now)
      .of_type(Signal::ARRIVAL);
}

bool Simulation::has_next(void) const {
  return queue.top().value().time <= settings.time_units;
}

double Simulation::blocking(void) const {
  return accepted_requests / (blocked_requests + accepted_requests);
}

double Simulation::entropy(void) const {
  auto free_bandwidth = 0.0;

  auto occupied_bandwidth = 0.0;

  for (const auto& [source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    const auto spectrum = hashmap.at(key);

    free_bandwidth += spectrum.available();

    occupied_bandwidth += spectrum.size() - spectrum.available();
  }

  if (free_bandwidth == 0.0 || occupied_bandwidth == 0.0) {
    return 0.0;
  }

  free_bandwidth /= settings.bandwidth;

  occupied_bandwidth /= settings.bandwidth;

  return -(occupied_bandwidth * std::log2(occupied_bandwidth) +
           free_bandwidth * std::log2(free_bandwidth));
}

double Simulation::fragmentation(void) const {
  auto fragmentation{0.0};

  for (const auto& [source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    fragmentation += hashmap.at(key).fragmentation();
  }

  return (fragmentation / settings.graph.get_edges().size());
}

std::vector<Snapshot> Simulation::get_snapshots(void) const {
  return snapshots;
}
