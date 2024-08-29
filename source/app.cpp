#include <cassert>
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
  auto hashmap{make_hashmap(settings.graph, settings.channels)};

  EventQueue<Connection> queue{settings.arrival_rate, settings.service_rate,
                               settings.seed};

  Group group{settings.seed, {100.0}, {1}};

  Timer timer;

  timer.start();

  double simulation_time{};

  queue.push(Connection{settings.graph.random_path(), group.next()}, 0.0)
      .of_type(Signal::ARRIVAL);

  while (group.size() < settings.calls) {
    const auto top{queue.pop()};

    auto [now, signal, connection] = top.value();

    simulation_time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::DEPARTURE) {
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

    assert(connection.slots != 0);

    if (!make_connection(connection, hashmap, settings.spectrum_allocator)) {
      group.count_blocking(connection.slots);

      INFO("Blocking request for " + std::to_string(connection.slots) +
           " slots");
    } else {
      queue.push(connection, now).of_type(Signal::DEPARTURE);

      INFO("Accept request for " + std::to_string(connection.slots) + " slots");

      const auto keys{path_keys(connection.path)};

      for (const auto &key : keys) {
        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }
    }

    queue.push(Connection{settings.graph.random_path(), group.next()}, now)
        .of_type(Signal::ARRIVAL);
  }

  timer.stop();

  std::string str{""};

  str.append("Execution time: " +
             std::to_string(timer.elapsed<std::chrono::seconds>()) + "s\n");

  str.append("Simulation time: " + std::to_string(simulation_time) + "\n");

  str.append(Report::from(group, settings).to_string() + "\n");

  return str;
}
