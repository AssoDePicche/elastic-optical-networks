#include "normal.h"

namespace prng {
Normal::Normal(const double mean, const double deviation)
    : _distribution{mean, deviation} {}

double Normal::Next(std::mt19937& generator) {
  return _distribution(generator);
}

}  // namespace prng
