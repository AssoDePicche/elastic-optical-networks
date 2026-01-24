#pragma once

#include <random>

#include "distribution.h"

namespace prng {
class Exponential final : public Distribution {
  std::exponential_distribution<double> _distribution;

 public:
  Exponential(const double);

  [[nodiscard]] double Next(std::mt19937&) override;
};
}  // namespace prng
