#include "math.h"

unsigned CantorPairingFunction(unsigned x, unsigned y) {
  return ((x + y) * (x + y + 1) / 2) + y;
}

Unit::Unit(double mean, double stddev, double variance)
    : mean{mean}, stddev{stddev}, variance{variance} {}
