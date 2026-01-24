#include "cantor.h"

namespace hash {
uint64_t CantorPairingFunction(uint64_t x, uint64_t y) {
  return ((x + y) * (x + y + 1) / 2) + y;
}

}  // namespace hash
