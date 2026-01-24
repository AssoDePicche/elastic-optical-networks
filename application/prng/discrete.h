#pragma once

#include <random>

#include "distribution.h"

namespace prng {
class Discrete final : public Distribution {
  std::discrete_distribution<int> _distribution;

 public:
  template <typename Iterator>
  Discrete(Iterator begin, Iterator end) : _distribution{begin, end} {}

  [[nodiscard]] double Next(std::mt19937&) override;
};

}  // namespace prng
