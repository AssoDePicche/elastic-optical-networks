#include <cassert>
#include <iostream>
#include <string>

#include "event_queue.h"
#include "graph.h"
#include "group.h"
#include "logger.h"
#include "spectrum.h"

struct Connection {
  Path path{};
  std::size_t slots{};
  std::size_t start{};
  std::size_t end{};

  Connection(void) = default;

  Connection(const Path &path, const std::size_t slots)
      : path{path}, slots{slots} {}
};

auto main(void) -> int {
  const auto channels{21u};
  const auto calls{1e3};
  const auto traffic_intensity{40.0};
  const auto arrival_rate{10};
  const auto service_rate{traffic_intensity / calls};

  Graph graph{2};

  graph.add_edge(0, 1, 1);

  graph.add_edge(1, 0, 1);

  Spectrum spectrum{channels};

  EventQueue<Connection> queue{arrival_rate, service_rate};

  Group group{{50.0, 50.0}, {3, 7}};

  const Connection first{graph.random_path(), group.next()};

  queue.push(first, 0.0).of_type(Signal::ARRIVAL);

  while (group.size() < calls) {
    const auto top{queue.pop()};

    assert(top.has_value());

    auto [now, signal, connection] = top.value();

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::BLOCKING) {
      group.count_blocking(connection.slots);

      continue;
    }

    if (signal == Signal::DEPARTURE) {
      spectrum.deallocate(connection.start, connection.end);

      INFO("Request for " + std::to_string(connection.slots) +
           " slots departing");

      INFO(spectrum.to_string());

      INFO("A (u): " + std::to_string(spectrum.available()) +
           " F (%): " + std::to_string(spectrum.fragmentation()));

      continue;
    }

    assert(signal == Signal::ARRIVAL);

    assert(connection.slots != 0);

    const auto search{first_fit(spectrum, connection.slots)};

    if (!search.has_value()) {
      queue.push(connection, now).of_type(Signal::BLOCKING);

      INFO("Blocking request for " + std::to_string(connection.slots) +
           " slots");
    } else {
      const auto start{search.value()};

      connection.start = start;

      connection.end = start + connection.slots - 1;

      spectrum.allocate(connection.start, connection.end);

      queue.push(connection, now).of_type(Signal::DEPARTURE);

      INFO("Accept request for " + std::to_string(connection.slots) + " slots");

      INFO(spectrum.to_string());

      INFO("A (u): " + std::to_string(spectrum.available()) +
           " F (%): " + std::to_string(spectrum.fragmentation()));
    }

    Connection next{graph.random_path(), group.next()};

    queue.push(next, now).of_type(Signal::ARRIVAL);
  }

  std::string str{};

  str.append("Traffic Intensity (E): " + std::to_string(traffic_intensity) +
             "\n");

  str.append("Service rate (μ): " + std::to_string(service_rate) + "\n");

  str.append("Arrival rate (λ): " + std::to_string(arrival_rate) + "\n");

  str.append("Channels: " + std::to_string(channels) + "\n");

  str.append("Calls: " + std::to_string(calls) + "\n");

  const auto GoS{static_cast<double>(group.blocked()) / calls};

  str.append("Grade of Service (%): " + std::to_string(GoS) + "\n");

  str.append(group.to_string());

  std::cout << str << std::endl;
};
