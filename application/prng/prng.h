#pragma once

#include <memory>
#include <random>
#include <string>
#include <unordered_map>

#include "discrete.h"
#include "distribution.h"

namespace prng {
class PseudoRandomNumberGenerator final {
  std::unordered_map<std::string, std::unique_ptr<Distribution>> _distribution;
  std::mt19937 _generator;
  std::random_device _random_device;
  uint64_t _seed;

 public:
  static std::shared_ptr<PseudoRandomNumberGenerator> Instance(void);

  [[nodiscard]] uint64_t GetSeed(void) const;

  void SetSeed(const uint64_t);

  void SetRandomSeed(void);

  void SetExponentialVariable(const std::string, const double);

  void SetPoissonVariable(const std::string, const double);

  void SetNormalVariable(const std::string, const double, const double);

  template <typename Iterator>
  void SetDiscreteVariable(const std::string key, Iterator begin,
                           Iterator end) {
    _distribution[key] = std::make_unique<Discrete>(begin, end);
  }

  void SetUniformVariable(const std::string, const double, const double);

  [[nodiscard]] double Next(const std::string);
};
}  // namespace prng
