#include "poisson.h"

namespace prng {
Poisson::Poisson(const double mean) : _distribution{mean} {}

double Poisson::Next(std::mt19937& generator) {
  return static_cast<double>(_distribution(generator));
}

}  // namespace prng
