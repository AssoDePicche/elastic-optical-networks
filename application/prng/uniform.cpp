#include "uniform.h"

namespace prng {
Uniform::Uniform(const double min, const double max)
    : _distribution{min, max} {}

double Uniform::Next(std::mt19937& generator) {
  return _distribution(generator);
}
}  // namespace prng
