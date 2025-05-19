#include "math.h"

unsigned CantorPairingFunction(unsigned x, unsigned y) {
  return ((x + y) * (x + y + 1) / 2) + y;
}
