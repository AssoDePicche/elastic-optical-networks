#include "group.h"

#include <cassert>

Group::Group(const unsigned seed,
             const std::initializer_list<double> &predicted,
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
