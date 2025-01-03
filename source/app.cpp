#include <cassert>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

#include "event_queue.h"
#include "group.h"
#include "logger.h"
#include "request.h"
#include "settings.h"
#include "statistics.h"
#include "timer.h"

struct Snapshot {
  float time;
  int slots;
  bool accepted;
  float fragmentation;
  float entropy;
  float blocking;

  Snapshot(float t, int slots, bool acc, float frag, float entr, float blocking)
      : time{t},
        slots{slots},
        accepted{acc},
        fragmentation{frag},
        entropy{entr},
        blocking{blocking} {}

  [[nodiscard]] auto str() const -> std::string {
    return std::to_string(time) + ", " + std::to_string(slots) + ", " +
           (accepted ? "Accepted" : "Blocked") + ", " +
           std::to_string(fragmentation) + ", " + std::to_string(entropy) +
           ", " + std::to_string(blocking);
  }
};

[[nodiscard]] auto simulation(Settings &) -> std::string;

auto main(const int argc, const char **argv) -> int {
  std::vector<std::string> args;

  for (auto arg{1u}; arg < static_cast<std::size_t>(argc); ++arg) {
    args.push_back(std::string(argv[arg]));
  }

  const auto typed_settings{Settings::from(args)};

  if (!typed_settings.has_value()) {
    return 1;
  }

  auto settings{typed_settings.value()};

  std::cout << simulation(settings) << std::endl;
}

auto simulation(Settings &settings) -> std::string {
  auto active_calls{0u};

  auto hashmap{make_hashmap(settings.graph, settings.channels)};

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

    free_slots /= settings.channels;

    occupied_slots /= settings.channels;

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

  queue.push(Request{settings.graph.random_path(), group.next()}, time)
      .of_type(Signal::ARRIVAL);

  while (queue.top().value().time <= settings.time_units) {
    const auto top{queue.pop()};

    auto [now, signal, request] = top.value();

    time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::DEPARTURE) {
      --active_calls;

      INFO("Request for " + std::to_string(request.slots) + " slots departing");

      const auto keys{path_keys(request.path)};

      for (const auto &key : keys) {
        hashmap[key].deallocate(request.slice);

        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

      continue;
    }

    const auto allocator = (request.slots == 3) ? first_fit : last_fit;

    auto accepted = false;

    if (active_calls < settings.channels &&
        dispatch_request(request, hashmap, allocator)) {
      ++active_calls;

      queue.push(request, now).of_type(Signal::DEPARTURE);

      INFO("Accept request for " + std::to_string(request.slots) + " slots");

      const auto keys{path_keys(request.path)};

      for (const auto &key : keys) {
        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

      accepted = true;

    } else {
      INFO("Blocking request for " + std::to_string(request.slots) + " slots");

      group.count_blocking(request.slots);
    }

    snapshots.push_back(Snapshot(now, request.slots, accepted,
                                 network_fragmentation(), entropy(),
                                 group.blocking()));

    queue.push(Request{settings.graph.random_path(), group.next()}, now)
        .of_type(Signal::ARRIVAL);
  }

  timer.stop();

  std::vector<double> fragmentation_states{};

  std::vector<double> entropy_states{};

  for (const auto &snapshot : snapshots) {
    std::cout << snapshot.str() << std::endl;

    fragmentation_states.push_back(snapshot.fragmentation);

    entropy_states.push_back(snapshot.entropy);
  }

  std::string str{""};

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
