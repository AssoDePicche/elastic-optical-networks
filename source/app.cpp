#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include <cmath>

#include "connection.h"
#include "event_queue.h"
#include "group.h"
#include "logger.h"
#include "settings.h"
#include "timer.h"

[[nodiscard]] auto mean(const std::vector<float>& X) -> float {
  auto sum = 0.0;

  for (const auto& x : X) {
    sum += x;
  }

  return (sum / X.size());
}

[[nodiscard]] auto stddev(const std::vector<float>& X) -> float {
  auto variance = 0.0;

  const auto x_mean = mean(X);

  for (const auto& x : X) {
    variance += ((x - x_mean) * (x - x_mean));
  }

  return std::sqrt((variance / X.size()));
}

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

  EventQueue<Connection> queue{settings.arrival_rate, settings.service_rate,
                               settings.seed};

  Group group{settings.seed, {50.0, 50.0}, {3, 5}};

  std::vector<float> fragmentation_states{};

  std::vector<float> entropy_states{};

  const auto entropy = [&](void) {
    const auto paths{settings.graph.paths()};

    auto free_slots = 0.0;

    auto occupied_slots = 0.0;

    for (const auto &path : paths) {
      for (const auto &key : path_keys(path)) {
        auto spectrum = hashmap[key];

        free_slots += spectrum.available();

        occupied_slots += spectrum.size() - spectrum.available();
      }
    }

    if (free_slots == 0.0 || occupied_slots == 0.0) {
      return 0.0;
    }

    free_slots /= settings.channels;

    occupied_slots /= settings.channels;

    return -(occupied_slots * std::log2(occupied_slots) + free_slots * std::log2(free_slots));
  };

  const auto network_fragmentation = [&](void) {
    const auto paths{settings.graph.paths()};

    auto fragmentation{0.0};

    for (const auto &path : paths) {
      for (const auto &key : path_keys(path)) {
        fragmentation += hashmap[key].fragmentation();
      }
    }

    return (fragmentation / paths.size());
  };

  Timer timer;

  timer.start();

  auto time{0.0};

  queue.push(Connection{settings.graph.random_path(), group.next()}, time)
      .of_type(Signal::ARRIVAL);

  while (queue.top().value().time <= settings.time_units) {
    fragmentation_states.push_back(network_fragmentation());

    entropy_states.push_back(entropy());

    const auto top{queue.pop()};

    auto [now, signal, connection] = top.value();

    time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::DEPARTURE) {
      --active_calls;

      INFO("Request for " + std::to_string(connection.slots) +
           " slots departing");

      const auto keys{path_keys(connection.path)};

      for (const auto &key : keys) {
        hashmap[key].deallocate(connection.start, connection.end);

        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

      continue;
    }

    if (active_calls < settings.channels &&
        make_connection(connection, hashmap, settings.spectrum_allocator)) {
      ++active_calls;

      queue.push(connection, now).of_type(Signal::DEPARTURE);

      INFO("Accept request for " + std::to_string(connection.slots) + " slots");

      const auto keys{path_keys(connection.path)};

      for (const auto &key : keys) {
        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

    } else {
      INFO("Blocking request for " + std::to_string(connection.slots) +
           " slots");

      group.count_blocking(connection.slots);
    }

    queue.push(Connection{settings.graph.random_path(), group.next()}, now)
        .of_type(Signal::ARRIVAL);
  }

  timer.stop();

  std::string str{""};

  str.append("Execution time: " +
             std::to_string(timer.elapsed<std::chrono::seconds>()) + "s\n");

  str.append("Simulation time: " + std::to_string(time) + "\n");

  str.append(Report::from(group, settings).to_string() + "\n");

  str.append("Mean fragmentation: " + std::to_string(mean(fragmentation_states)) + "\n");

  str.append("STDDEV fragmentation: " + std::to_string(stddev(fragmentation_states)) + "\n");

  str.append("Mean entropy: " + std::to_string(mean(entropy_states)) + "\n");

  str.append("STDDEV entropy: " + std::to_string(stddev(entropy_states)) + "\n");

  return str;
}
