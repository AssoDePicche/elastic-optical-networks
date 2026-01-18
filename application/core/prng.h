#pragma once

#include <memory>
#include <random>
#include <string>
#include <unordered_map>

namespace core {
class Distribution {
 public:
  virtual ~Distribution();

  [[nodiscard]] virtual double next(std::mt19937&) = 0;
};

class Exponential final : public Distribution {
  std::exponential_distribution<double> _distribution;

 public:
  Exponential(const double);

  [[nodiscard]] double next(std::mt19937&) override;
};

class Poisson final : public Distribution {
  std::poisson_distribution<int> _distribution;

 public:
  Poisson(const double);

  [[nodiscard]] double next(std::mt19937&) override;
};

class Normal final : public Distribution {
  std::normal_distribution<double> _distribution;

 public:
  Normal(const double, const double);

  [[nodiscard]] double next(std::mt19937&) override;
};

class Discrete final : public Distribution {
  std::discrete_distribution<int> _distribution;

 public:
  template <typename Iterator>
  Discrete(Iterator begin, Iterator end) : _distribution{begin, end} {}

  [[nodiscard]] double next(std::mt19937&) override;
};

class Uniform final : public Distribution {
  std::uniform_real_distribution<double> _distribution;

 public:
  Uniform(const double, const double);

  [[nodiscard]] double next(std::mt19937&) override;
};

class PseudoRandomNumberGenerator final {
  std::unordered_map<std::string, std::unique_ptr<Distribution>> _distribution;
  std::mt19937 _generator;
  std::random_device _random_device;
  uint64_t _seed;

 public:
  static std::shared_ptr<PseudoRandomNumberGenerator> Instance(void);

  [[nodiscard]] uint64_t seed(void) const;

  void seed(const uint64_t);

  void random_seed(void);

  void exponential(const std::string, const double);

  void poisson(const std::string, const double);

  void normal(const std::string, const double, const double);

  template <typename Iterator>
  void discrete(const std::string key, Iterator begin, Iterator end) {
    _distribution[key] = std::make_unique<Discrete>(begin, end);
  }

  void uniform(const std::string, const double, const double);

  [[nodiscard]] double next(const std::string);
};
}  // namespace core
