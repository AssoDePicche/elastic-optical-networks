#include "path.h"

#include <hash/cantor.h>

#include <algorithm>
#include <ranges>

namespace core {
bool Path::empty() const noexcept { return vertices.empty(); }

std::vector<uint64_t> Path::keys() const noexcept {
  std::vector<uint64_t> keys;

  for (const auto& index : std::views::iota(1u, vertices.size())) {
    const auto x = *std::next(vertices.begin(), index - 1);

    const auto y = *std::next(vertices.begin(), index);

    const auto key = hash::CantorPairingFunction(x, y);

    keys.push_back(key);
  }

  return keys;
}

bool Path::operator>(const Path& other) const { return cost > other.cost; }
}  // namespace core
