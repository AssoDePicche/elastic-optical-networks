#pragma once

#include <random>

namespace prng {
class Distribution {
 public:
  virtual ~Distribution();

  [[nodiscard]] virtual double Next(std::mt19937&) = 0;
};
}  // namespace prng
