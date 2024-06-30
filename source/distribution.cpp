#include "distribution.h"

Exponential::Exponential(const double mean) : distribution{mean} {}

auto Exponential::next(void) -> double { return distribution(generator); }

Poisson::Poisson(const double mean) : distribution{mean} {}

auto Poisson::next(void) -> double {
  return static_cast<double>(distribution(generator));
}

Normal::Normal(const double mean, const double deviation)
    : distribution{mean, deviation} {}

auto Normal::next(void) -> double { return distribution(generator); }

Discrete::Discrete(const std::initializer_list<double> &list)
    : distribution{list} {}

auto Discrete::next(void) -> double {
  return static_cast<double>(distribution(generator));
}

Uniform::Uniform(const double min, const double max) : distribution{min, max} {}

auto Uniform::next(void) -> double { return distribution(generator); }
