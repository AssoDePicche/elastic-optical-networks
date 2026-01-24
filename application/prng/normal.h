#pragma once

#include <random>

#include "distribution.h"

namespace prng {
class Normal final : public Distribution {
  std::normal_distribution<double> _distribution;

 public:
  Normal(const double, const double);

  [[nodiscard]] double Next(std::mt19937&) override;
};

}  // namespace prng
