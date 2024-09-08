#include <cassert>
#include <fstream>
#include <iostream>
#include <string>

#include "connection.h"
#include "event_queue.h"
#include "group.h"
#include "logger.h"
#include "settings.h"
#include "timer.h"

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

  auto iterations{0u};

  auto hashmap{make_hashmap(settings.graph, settings.channels)};

  EventQueue<Connection> queue{settings.arrival_rate, settings.service_rate,
                               settings.seed};

  Group group{settings.seed, {50.0, 50.0}, {3, 5}};

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

  std::vector<std::string> features{};

  std::vector<bool> labels{};

  const auto state = [&](const Connection &c, const bool blocked) {
    auto a{0u};

    const auto k{path_keys(c.path)};

    for (const auto &key : k) {
      a += hashmap[key].smallest_partition();
    }

    std::string str{""};

    str.append(std::to_string(c.slots == 3 ? 0 : 1) + ",");

    str.append(std::to_string(c.slots) + ",");

    str.append(std::to_string(static_cast<double>(a) / k.size()) + ",");

    // TODO: get fragmentation from connection path
    str.append(std::to_string(network_fragmentation()));

    features.push_back(str);

    labels.push_back(blocked);
  };

  auto total_fragmentation{0.0};

  Timer timer;

  timer.start();

  auto time{0.0};

  queue.push(Connection{settings.graph.random_path(), group.next()}, time)
      .of_type(Signal::ARRIVAL);

  while (queue.top().value().time <= settings.time_units) {
    ++iterations;

    total_fragmentation += network_fragmentation();

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

      state(connection, true);

      const auto keys{path_keys(connection.path)};

      for (const auto &key : keys) {
        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

    } else {
      state(connection, false);

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

  str.append("Mean fragmentation: " +
             std::to_string(total_fragmentation / iterations) + "\n");

  std::ofstream trainingSet{"training-set.csv"};

  for (const auto &row : features) {
    trainingSet << row << '\n';
  }

  trainingSet.close();

  std::ofstream labelSet{"label-set.csv"};

  for (const auto &row : labels) {
    labelSet << row << '\n';
  }

  labelSet.close();

  return str;
}
