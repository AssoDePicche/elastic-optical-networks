#include "simulation.h"

#include "event_queue.h"
#include "graph.h"
#include "group.h"
#include "logger.h"
#include "request.h"
#include "statistics.h"
#include "timer.h"

Snapshot::Snapshot(float t, int slots, bool acc, float frag, float entr,
                   float blocking)
    : time{t},
      slots{slots},
      accepted{acc},
      fragmentation{frag},
      entropy{entr},
      blocking{blocking} {}

auto Snapshot::str(void) const -> std::string {
  return std::to_string(time) + ", " + std::to_string(slots) + ", " +
         (accepted ? "True" : "False") + ", " + std::to_string(fragmentation) +
         ", " + std::to_string(entropy) + ", " + std::to_string(blocking);
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
    --active_calls;

    INFO("Request for " + std::to_string(request.bandwidth) +
         " slots departing");

    const auto keys{route_keys(request.route)};

    for (const auto& key : keys) {
      hashmap[key].deallocate(request.slice);

      INFO(hashmap[key].to_string());

      INFO("A (u): " + std::to_string(hashmap[key].available()) +
           " F (%): " + std::to_string(hashmap[key].fragmentation()));
    }

    return;
  }

  const auto allocator = (request.bandwidth == 3) ? first_fit : last_fit;

  auto accepted = false;

  if (active_calls < settings.bandwidth &&
      dispatch_request(request, hashmap, allocator)) {
    ++active_calls;

    queue.push(request, now).of_type(Signal::DEPARTURE);

    INFO("Accept request for " + std::to_string(request.bandwidth) + " slots");

    const auto keys{route_keys(request.route)};

    for (const auto& key : keys) {
      INFO(hashmap[key].to_string());

      INFO("A (u): " + std::to_string(hashmap[key].available()) +
           " F (%): " + std::to_string(hashmap[key].fragmentation()));
    }

    accepted = true;

  } else {
    INFO("Blocking request for " + std::to_string(request.bandwidth) +
         " slots");

    group.count_blocking(request.bandwidth);
  }

  snapshots.push_back(Snapshot(now, request.bandwidth, accepted,
                               fragmentation(), entropy(), group.blocking()));

  queue.push(Request{random_path(settings.graph), group.next()}, now)
      .of_type(Signal::ARRIVAL);
}

bool Simulation::has_next(void) const {
  return queue.top().value().time <= settings.time_units;
}

double Simulation::entropy(void) const {
  auto free_slots = 0.0;

  auto occupied_slots = 0.0;

  for (const auto& [source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    const auto spectrum = hashmap.at(key);

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
