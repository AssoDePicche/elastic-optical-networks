#pragma once

#include <random>

#include "distribution.h"

namespace prng {
class Uniform final : public Distribution {
  std::uniform_real_distribution<double> _distribution;

 public:
  Uniform(const double, const double);

  [[nodiscard]] double Next(std::mt19937&) override;
};

}  // namespace prng
