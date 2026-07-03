#pragma once

#include <cstdint>

namespace core {
struct Lightpath final {
  uint64_t pathIndex;
  uint64_t coreIndex;
  uint64_t start;
  uint64_t end;
};
}  // namespace core
