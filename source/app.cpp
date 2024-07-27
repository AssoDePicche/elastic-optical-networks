#include <cassert>
#include <iostream>
#include <map>
#include <string>

#include "event_queue.h"
#include "graph.h"
#include "group.h"
#include "logger.h"
#include "parser.h"
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
  std::vector<std::string> args{"--calls",    "--channels",
                                "--erlangs",  "--lambda",
                                "--topology", "--spectrum-allocator"};
  if (argc < 2) {
    std::cerr << "You must pass all the arguments:" << std::endl;

    for (const auto &arg : args) {
      std::cerr << arg << std::endl;
    }

    return 1;
  }

  const Parser parser{argc, argv};

  for (const auto &arg : args) {
    if (!parser.contains(arg)) {
      std::cerr << "You must pass the " << arg << " argument" << std::endl;

      return 1;
    }
  }

  auto str_to_size_t = [](const std::string &str) {
    return static_cast<std::size_t>(std::atoi(str.c_str()));
  };

  auto str_to_double = [](const std::string &str) { return std::stod(str); };

  const auto channels{str_to_size_t(parser.parse("--channels"))};

  assert(channels > 0u);

  const auto calls{str_to_size_t(parser.parse("--calls"))};

  assert(calls > 0u);

  const auto traffic_intensity{str_to_double(parser.parse("--erlangs"))};

  assert(traffic_intensity > 0.0);

  const auto arrival_rate{str_to_double(parser.parse("--lambda"))};

  assert(arrival_rate > 0.0);

  const auto service_rate{traffic_intensity / arrival_rate};

  assert(service_rate > 0.0);

  const auto filename{parser.parse("--topology")};

  const auto container{Graph::from(filename)};

  if (!container.has_value()) {
    std::cerr << "Unable to read the " << filename << " file." << std::endl;

    return 1;
  }

  auto graph{container.value()};

  const auto spectrum_allocation_strategy{parser.parse("--spectrum-allocator")};

  const std::map<std::string, SpectrumAllocator> spectrum_allocation_strategies{
      {"best-fit", best_fit},
      {"first-fit", first_fit},
      {"last-fit", last_fit},
      {"random-fit", random_fit},
      {"worst-fit", worst_fit}};

  assert(spectrum_allocation_strategies.find(spectrum_allocation_strategy) !=
         spectrum_allocation_strategies.end());

  const auto spectrum_allocator{
      spectrum_allocation_strategies.at(spectrum_allocation_strategy)};

  auto hashmap{make_hashmap(graph, channels)};

  EventQueue<Connection> queue{arrival_rate, service_rate};

  Group group{{50.0, 50.0}, {3, 7}};

  Timer timer;

  timer.start();

  Time simulation_time;

  const Connection first{graph.random_path(), group.next()};

  queue.push(first, 0.0).of_type(Signal::ARRIVAL);

  while (group.size() < calls) {
    const auto top{queue.pop()};

    assert(top.has_value());

    auto [now, signal, connection] = top.value();

    simulation_time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::BLOCKING) {
      group.count_blocking(connection.slots);

      continue;
    }

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

    assert(signal == Signal::ARRIVAL);

    assert(connection.slots != 0);

    if (!make_connection(connection, hashmap, spectrum_allocator)) {
      queue.push(connection, now).of_type(Signal::BLOCKING);

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

    Connection next{graph.random_path(), group.next()};

    queue.push(next, now).of_type(Signal::ARRIVAL);
  }

  timer.stop();

  std::string str{};

  const auto real_time{timer.elapsed<std::chrono::seconds>()};

  const auto utilization{arrival_rate / service_rate};

  const auto service_duration{1.0 / service_rate};

  const auto GoS{static_cast<double>(group.blocked()) / calls};

  const auto busy_channels{(1.0 - GoS) * traffic_intensity};

  const auto occupancy{busy_channels / channels};

  str.append("Execution time: " + std::to_string(real_time) + "s\n");

  str.append("Simulation time: " + std::to_string(simulation_time) + "\n");

  str.append("Channels (C): " + std::to_string(channels) + "\n");

  str.append("Calls (n): " + std::to_string(calls) + "\n");

  str.append("Traffic Intensity (E): " + std::to_string(traffic_intensity) +
             "\n");

  str.append("Arrival rate (λ): " + std::to_string(arrival_rate) + "\n");

  str.append("Service rate (μ): " + std::to_string(service_rate) + "\n");

  str.append("Duration of service (1/μ): " + std::to_string(service_duration) +
             "\n");

  str.append("Utilization (ρ): " + std::to_string(utilization) + "\n");

  str.append("Idle rate (1-ρ): " + std::to_string(1.0 - utilization) + "\n");

  str.append("Grade of Service (ε): " + std::to_string(GoS) + "\n");

  str.append("Busy Channels (1-ε): " + std::to_string(busy_channels) + "\n");

  str.append("Occupancy ((1-ε)/C): " + std::to_string(occupancy) + "\n");

  str.append(group.to_string());

  std::cout << str << std::endl;
};
