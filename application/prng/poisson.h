#pragma once

#include <random>

#include "distribution.h"

namespace prng {
class Poisson final : public Distribution {
  std::poisson_distribution<int> _distribution;

 public:
  Poisson(const double);

  [[nodiscard]] double Next(std::mt19937&) override;
};
}  // namespace prng
