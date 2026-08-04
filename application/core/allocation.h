#pragma once

#include <cstdint>

#include "flexgrid.h"

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
};
