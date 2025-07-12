#pragma once

#include <initializer_list>
#include <random>
#include <vector>

using Seed = long long unsigned;

class Distribution {
 public:
  Distribution(const Seed);

  virtual auto next(void) -> double = 0;

  [[nodiscard]] auto get_seed(void) const -> Seed;

 private:
  Seed seed;

 protected:
  std::mt19937 generator;
};

class Exponential final : public Distribution {
 public:
  Exponential(const Seed, const double);

  auto next(void) -> double override;

 private:
  std::exponential_distribution<double> distribution;
};

class Poisson final : public Distribution {
 public:
  Poisson(const Seed, const double);

  auto next(void) -> double override;

 private:
  std::poisson_distribution<int> distribution;
};

class Normal final : public Distribution {
 public:
  Normal(const Seed, const double, const double);

  auto next(void) -> double override;

 private:
  std::normal_distribution<double> distribution;
};

class Discrete final : public Distribution {
 public:
  Discrete(const Seed, const std::vector<double> &);

  auto next(void) -> double override;

 private:
  std::discrete_distribution<int> distribution;
};

class Uniform final : public Distribution {
 public:
  Uniform(const Seed, const double, const double);

  auto next(void) -> double override;

 private:
  std::uniform_real_distribution<double> distribution;
};
