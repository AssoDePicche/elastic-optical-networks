#include <cassert>
#include <iostream>
#include <map>
#include <string>

#include "event_queue.h"
#include "graph.h"
#include "group.h"
#include "logger.h"
#include "parser.h"
#include "settings.h"
#include "spectrum.h"
#include "timer.h"

struct Connection {
  Path path{};
  std::size_t slots{};
  std::size_t start{};
  std::size_t end{};

  Connection(void) = default;

  Connection(const Path &path, const std::size_t slots)
      : path{path}, slots{slots} {}
};

auto make_key(std::size_t x, std::size_t y) -> std::size_t {
  return ((x + y) * (x + y + 1) / 2) + y;
}

auto path_keys(const Path &path) -> std::vector<std::size_t> {
  assert(!path.empty());

  std::vector<std::size_t> keys;

  for (auto index{1u}; index < path.size(); ++index) {
    keys.push_back(make_key(path[index - 1], path[index]));
  }

  return keys;
}

auto make_hashmap(const Graph &graph, const std::size_t size)
    -> std::map<std::size_t, Spectrum> {
  std::map<std::size_t, Spectrum> hashmap;

  for (auto source{0u}; source < graph.size(); ++source) {
    for (auto destination{0u}; destination < graph.size(); ++destination) {
      if (graph.at(source, destination) != __MIN_WEIGHT__) {
        const auto key{make_key(source, destination)};

        hashmap[key] = Spectrum(size);
      }
    }
  }

  return hashmap;
}

auto make_connection(Connection &connection,
                     std::map<std::size_t, Spectrum> &hashmap,
                     const SpectrumAllocator &spectrum_allocator) -> bool {
  const auto keys{path_keys(connection.path)};

  const auto search{spectrum_allocator(hashmap[keys[0]], connection.slots)};

  if (!search.has_value()) {
    return false;
  }

  connection.start = search.value();

  connection.end = connection.start + connection.slots - 1;

  for (const auto &key : keys) {
    if (!hashmap[key].available_at(connection.start, connection.end)) {
      return false;
    }
  }

  for (const auto &key : keys) {
    hashmap[key].allocate(connection.start, connection.end);
  }

  return true;
}

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

  auto hashmap{make_hashmap(settings.graph, settings.channels)};

  auto arrival_distribution{
      std::make_shared<Exponential>(settings.seed, settings.arrival_rate)};

  auto service_distribution{
      std::make_shared<Exponential>(settings.seed, settings.service_rate)};

  EventQueue<Connection> queue{arrival_distribution, service_distribution};

  Group group{settings.seed, {50.0, 50.0}, {3, 7}};

  Timer timer;

  timer.start();

  Time simulation_time;

  const Connection first{settings.graph.random_path(), group.next()};

  queue.push(first, 0.0).of_type(Signal::ARRIVAL);

  while (group.size() < settings.calls) {
    const auto top{queue.pop()};

    assert(top.has_value());

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

    Connection next{settings.graph.random_path(), group.next()};

    queue.push(next, now).of_type(Signal::ARRIVAL);
  }

  timer.stop();

  const auto real_time{timer.elapsed<std::chrono::seconds>()};

  std::cout << "Execution time: " + std::to_string(real_time) + "s"
            << std::endl;

  std::cout << "Simulation time: " + std::to_string(simulation_time)
            << std::endl;

  std::cout << Report::from(group, settings).to_string() << std::endl;
};
