#include "distribution.h"

Distribution::Distribution(const unsigned seed) : seed{seed}, generator{seed} {}

auto Distribution::get_seed(void) const -> unsigned { return seed; }

Exponential::Exponential(const unsigned seed, const double mean)
    : Distribution(seed), distribution{mean} {}

auto Exponential::next(void) -> double { return distribution(generator); }

Poisson::Poisson(const unsigned seed, const double mean)
    : Distribution(seed), distribution{mean} {}

auto Poisson::next(void) -> double {
  return static_cast<double>(distribution(generator));
}

Normal::Normal(const unsigned seed, const double mean, const double deviation)
    : Distribution(seed), distribution{mean, deviation} {}

auto Normal::next(void) -> double { return distribution(generator); }

Discrete::Discrete(const unsigned seed,
                   const std::initializer_list<double> &list)
    : Distribution(seed), distribution{list} {}

auto Discrete::next(void) -> double {
  return static_cast<double>(distribution(generator));
}

Uniform::Uniform(const unsigned seed, const double min, const double max)
    : Distribution(seed), distribution{min, max} {}

auto Uniform::next(void) -> double { return distribution(generator); }
