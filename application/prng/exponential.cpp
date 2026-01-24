#include "exponential.h"

namespace prng {
Exponential::Exponential(const double mean) : _distribution{mean} {}

double Exponential::Next(std::mt19937& generator) {
  return _distribution(generator);
}

}  // namespace prng
