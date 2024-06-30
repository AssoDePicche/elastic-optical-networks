#pragma once

#include <initializer_list>
#include <random>

class Distribution {
public:
  virtual auto next(void) -> double = 0;

protected:
  std::random_device seed;
  std::mt19937 generator{seed()};
};

class Exponential final : public Distribution {
public:
  Exponential(const double);

  auto next(void) -> double override;

private:
  std::exponential_distribution<double> distribution;
};

class Poisson final : public Distribution {
public:
  Poisson(const double);

  auto next(void) -> double override;

private:
  std::poisson_distribution<int> distribution;
};

class Normal final : public Distribution {
public:
  Normal(const double, const double);

  auto next(void) -> double override;

private:
  std::normal_distribution<double> distribution;
};

class Discrete final : public Distribution {
public:
  Discrete(const std::initializer_list<double> &);

  auto next(void) -> double override;

private:
  std::discrete_distribution<int> distribution;
};

class Uniform final : public Distribution {
public:
  Uniform(const double, const double);

  auto next(void) -> double override;

private:
  std::uniform_real_distribution<double> distribution;
};
