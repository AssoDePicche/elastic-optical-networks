#pragma once

#include <cstdint>
#include <vector>

namespace core {
struct Allocation {
  uint64_t fiberIndex;
  uint64_t coreIndex;
  uint64_t start;
  uint64_t end;
};

using Lightpath = std::vector<Allocation>;
}  // namespace core
