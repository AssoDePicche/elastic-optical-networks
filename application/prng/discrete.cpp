#include "discrete.h"

namespace prng {
double Discrete::Next(std::mt19937& generator) {
  return static_cast<double>(_distribution(generator));
}
}  // namespace prng
