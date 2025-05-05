#include "group.h"

#include <cassert>

Group::Group(const Seed seed, const std::vector<double> &predicted,
             const std::vector<unsigned> &resources)
    : roulette{seed, predicted} {
  assert(predicted.size() == resources.size());

  for (auto index{0u}; index < predicted.size(); ++index) {
    const auto request{*(resources.begin() + index)};

    const auto value{std::make_tuple(request, 0, 0)};

    container.push_back(value);
  }
}

auto Group::count_blocking(const unsigned request) -> void {
  for (auto &[resource, counting, blocking] : container) {
    if (resource != request) {
      continue;
    }

    ++blocking;

    ++_blocking;

    break;
  }
}

auto Group::size(void) const -> unsigned { return _size; }

auto Group::blocked(void) const -> unsigned { return _blocking; }

auto Group::blocking(void) const -> double {
  return static_cast<double>(_blocking) / static_cast<double>(_size);
}

auto Group::next(void) -> unsigned {
  const auto index{static_cast<unsigned>(roulette.next())};

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

  report.bandwidth = settings.bandwidth;

  report.calls = group.size();

  report.arrival_rate = settings.arrival_rate;

  report.service_rate = settings.service_rate;

  report.traffic_intensity = (settings.arrival_rate / settings.service_rate);

  report.grade_of_service = static_cast<double>(group.blocked()) / report.calls;

  report.busy_channels = (1.0 - report.grade_of_service);

  report.occupancy = (report.busy_channels / report.bandwidth);

  return report;
}

auto Report::to_string(void) const -> std::string {
  std::string str{};

  str.append("Busy Channels (1-ε): " + std::to_string(busy_channels) + "\n");

  str.append("Occupancy ((1-ε)/C): " + std::to_string(occupancy) + "\n");

  str.append(group_str);

  return str;
}
