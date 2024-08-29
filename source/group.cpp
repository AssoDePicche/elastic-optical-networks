#include "group.h"

#include <cassert>

Group::Group(const Seed seed, const std::initializer_list<double> &predicted,
             const std::initializer_list<std::size_t> &resources)
    : roulette{seed, predicted} {
  assert(predicted.size() == resources.size());

  for (auto index{0u}; index < predicted.size(); ++index) {
    const auto request{*(resources.begin() + index)};

    const auto value{std::make_tuple(request, 0, 0)};

    container.push_back(value);
  }
}

auto Group::count_blocking(const std::size_t request) -> void {
  for (auto &[resource, counting, blocking] : container) {
    if (resource != request) {
      continue;
    }

    ++blocking;

    ++_blocking;

    break;
  }
}

auto Group::size(void) const -> std::size_t { return _size; }

auto Group::blocked(void) const -> std::size_t { return _blocking; }

auto Group::next(void) -> std::size_t {
  const auto index{static_cast<std::size_t>(roulette.next())};

  auto &[resource, counting, blocking] = container[index];

  ++counting;

  ++_size;

  return resource;
}

auto Group::to_string(void) const -> std::string {
  std::string buffer{};

  for (const auto &[resource, counting, blocking] : container) {
    std::string line{};

    line.append("Type " + std::to_string(resource) + "\n");

    const auto ratio{static_cast<double>(counting) / size()};

    line.append("Ratio: " + std::to_string(ratio) + "\n");

    const auto bp{static_cast<double>(blocking) / size()};

    line.append("BP(ε): " + std::to_string(bp) + "\n");

    buffer.append(line);
  }

  return buffer;
}

auto Report::from(const Group &group, const Settings &settings) -> Report {
  Report report;

  report.group_str = group.to_string();

  report.seed = settings.seed;

  report.channels = settings.channels;

  report.calls = settings.calls;

  report.arrival_rate = settings.arrival_rate;

  report.service_rate = settings.service_rate;

  report.traffic_intensity = settings.traffic_intensity;

  report.grade_of_service = static_cast<double>(group.blocked()) / report.calls;

  report.busy_channels =
      (1.0 - report.grade_of_service) * report.traffic_intensity;

  report.occupancy = report.busy_channels / report.channels;

  return report;
}

#include <iostream>

auto Report::to_string(void) const -> std::string {
  std::string str{};

  str.append("Seed: " + std::to_string(seed) + "\n");

  str.append("Channels (C): " + std::to_string(channels) + "\n");

  str.append("Calls (n): " + std::to_string(calls) + "\n");

  str.append("Arrival rate (λ): " + std::to_string(arrival_rate) + "\n");

  str.append("Service rate (μ): " + std::to_string(service_rate) + "\n");

  std::cout << service_rate << std::endl;

  str.append("Traffic Intensity (ρ): " + std::to_string(traffic_intensity) +
             "\n");

  str.append("Grade of Service (ε): " + std::to_string(grade_of_service) +
             "\n");

  str.append("Busy Channels (1-ε): " + std::to_string(busy_channels) + "\n");

  str.append("Occupancy ((1-ε)/C): " + std::to_string(occupancy) + "\n");

  str.append(group_str);

  return str;
}
