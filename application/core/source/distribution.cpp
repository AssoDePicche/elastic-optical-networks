#include "distribution.h"

Distribution::Distribution(const Seed seed) : seed{seed}, generator{seed} {}

auto Distribution::get_seed(void) const -> Seed { return seed; }

Exponential::Exponential(const Seed seed, const double mean)
    : Distribution(seed), distribution{mean} {}

auto Exponential::next(void) -> double { return distribution(generator); }

Poisson::Poisson(const Seed seed, const double mean)
    : Distribution(seed), distribution{mean} {}

auto Poisson::next(void) -> double {
  return static_cast<double>(distribution(generator));
}

Normal::Normal(const Seed seed, const double mean, const double deviation)
    : Distribution(seed), distribution{mean, deviation} {}

auto Normal::next(void) -> double { return distribution(generator); }

Discrete::Discrete(const Seed seed, const std::vector<double> &list)
    : Distribution(seed), distribution{list.begin(), list.end()} {}

auto Discrete::next(void) -> double {
  return static_cast<double>(distribution(generator));
}

Uniform::Uniform(const Seed seed, const double min, const double max)
    : Distribution(seed), distribution{min, max} {}

auto Uniform::next(void) -> double { return distribution(generator); }
