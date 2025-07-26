#pragma once

#include <memory>
#include <random>
#include <string>
#include <unordered_map>

class Distribution {
 public:
  Distribution(const uint64_t);

  [[nodiscard]] virtual double next(void) = 0;

  [[nodiscard]] uint64_t get_seed(void) const;

  void set_seed(const uint64_t);

 private:
  uint64_t seed;

 protected:
  std::mt19937 generator;
};

class Exponential final : public Distribution {
 public:
  Exponential(const uint64_t, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::exponential_distribution<double> distribution;
};

class Poisson final : public Distribution {
 public:
  Poisson(const uint64_t, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::poisson_distribution<int> distribution;
};

class Normal final : public Distribution {
 public:
  Normal(const uint64_t, const double, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::normal_distribution<double> distribution;
};

class Discrete final : public Distribution {
 public:
  template <typename Iterator>
  Discrete(const uint64_t seed, Iterator begin, Iterator end)
      : Distribution(seed), distribution{begin, end} {}

  [[nodiscard]] double next(void) override;

 private:
  std::discrete_distribution<int> distribution;
};

class Uniform final : public Distribution {
 public:
  Uniform(const uint64_t, const double, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::uniform_real_distribution<double> distribution;
};

class PseudoRandomNumberGenerator final {
 public:
  static std::shared_ptr<PseudoRandomNumberGenerator> Instance(void);

  [[nodiscard]] uint64_t get_seed(void) const;

  void set_seed(const uint64_t);

  void set_exponential(const std::string, const double);

  void set_poisson(const std::string, const double);

  void set_normal(const std::string, const double, const double);

  template <typename Iterator>
  void set_discrete(const std::string key, Iterator begin, Iterator end) {
    distribution[key] = std::make_unique<Discrete>(seed, begin, end);
  }

  void set_uniform(const std::string, const double, const double);

  [[nodiscard]] double next(const std::string);

 private:
  std::unordered_map<std::string, std::shared_ptr<Distribution>> distribution;
  uint64_t seed;
};
