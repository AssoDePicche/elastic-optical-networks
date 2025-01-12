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
         (accepted ? "true" : "false") + ", " + std::to_string(fragmentation) +
         ", " + std::to_string(entropy) + ", " + std::to_string(blocking);
}

auto simulation(Settings &settings) -> std::string {
  auto active_calls{0u};

  auto hashmap{make_hashmap(settings.graph, settings.bandwidth)};

  EventQueue<Request> queue{settings.arrival_rate, settings.service_rate,
                            settings.seed};

  Group group{settings.seed, {50.0, 50.0}, {3, 5}};

  std::vector<Snapshot> snapshots{};

  const auto entropy = [&](void) {
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
  };

  const auto network_fragmentation = [&](void) {
    auto fragmentation{0.0};

    for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
      const auto key = make_key(source, destination);

      fragmentation += hashmap[key].fragmentation();
    }

    return (fragmentation / settings.graph.get_edges().size());
  };

  Timer timer;

  timer.start();

  auto time{0.0};

  queue.push(Request{random_path(settings.graph), group.next()}, time)
      .of_type(Signal::ARRIVAL);

  while (queue.top().value().time <= settings.time_units) {
    const auto top{queue.pop()};

    auto [now, signal, request] = top.value();

    time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::DEPARTURE) {
      --active_calls;

      INFO("Request for " + std::to_string(request.bandwidth) +
           " slots departing");

      const auto keys{route_keys(request.route)};

      for (const auto &key : keys) {
        hashmap[key].deallocate(request.slice);

        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

      continue;
    }

    const auto allocator = (request.bandwidth == 3) ? first_fit : last_fit;

    auto accepted = false;

    if (active_calls < settings.bandwidth &&
        dispatch_request(request, hashmap, allocator)) {
      ++active_calls;

      queue.push(request, now).of_type(Signal::DEPARTURE);

      INFO("Accept request for " + std::to_string(request.bandwidth) +
           " slots");

      const auto keys{route_keys(request.route)};

      for (const auto &key : keys) {
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
                                 network_fragmentation(), entropy(),
                                 group.blocking()));

    queue.push(Request{random_path(settings.graph), group.next()}, now)
        .of_type(Signal::ARRIVAL);
  }

  timer.stop();

  std::string str{""};

  str.append("time,slots,accepted,fragmentation,entropy,blocking\n");

  std::vector<double> fragmentation_states;

  std::vector<double> entropy_states;

  for (const auto &snapshot : snapshots) {
    str.append(snapshot.str() + "\n");

    fragmentation_states.push_back(snapshot.fragmentation);

    entropy_states.push_back(snapshot.entropy);
  }

  str.append("Execution time: " +
             std::to_string(timer.elapsed<std::chrono::seconds>()) + "s\n");

  str.append("Simulation time: " + std::to_string(time) + "\n");

  str.append(Report::from(group, settings).to_string() + "\n");

  str.append("Mean fragmentation: " +
             std::to_string(MEAN(fragmentation_states)) + "\n");

  str.append("STDDEV fragmentation: " +
             std::to_string(STDDEV(fragmentation_states)) + "\n");

  str.append("Mean entropy: " + std::to_string(MEAN(entropy_states)) + "\n");

  str.append("STDDEV entropy: " + std::to_string(STDDEV(entropy_states)) +
             "\n");

  return str;
}
