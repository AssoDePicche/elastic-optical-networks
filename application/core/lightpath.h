#pragma once

#include <cstdint>
#include <vector>

namespace core {
struct Slice final {
  uint16_t firstFSU;
  uint16_t lastFSU;
};

struct Allocation final {
  Slice slice;
  uint16_t link;
  uint8_t core;
};

using Lightpath = std::vector<Allocation>;
}  // namespace core
