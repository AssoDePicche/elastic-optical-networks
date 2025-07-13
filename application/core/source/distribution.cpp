#include "distribution.h"

Distribution::Distribution(const Seed seed) : seed{seed}, generator{seed} {}

Seed Distribution::get_seed(void) const { return seed; }

void Distribution::set_seed(const Seed seed) {
  this->seed = seed;

  generator.seed(seed);
}

Exponential::Exponential(const Seed seed, const double mean)
    : Distribution(seed), distribution{mean} {}

double Exponential::next(void) { return distribution(generator); }

Poisson::Poisson(const Seed seed, const double mean)
    : Distribution(seed), distribution{mean} {}

double Poisson::next(void) {
  return static_cast<double>(distribution(generator));
}

Normal::Normal(const Seed seed, const double mean, const double deviation)
    : Distribution(seed), distribution{mean, deviation} {}

double Normal::next(void) { return distribution(generator); }

Discrete::Discrete(const Seed seed, const std::vector<double> &list)
    : Distribution(seed), distribution{list.begin(), list.end()} {}

double Discrete::next(void) {
  return static_cast<double>(distribution(generator));
}

Uniform::Uniform(const Seed seed, const double min, const double max)
    : Distribution(seed), distribution{min, max} {}

auto Uniform::next(void) -> double { return distribution(generator); }

std::shared_ptr<PseudoRandomNumberGenerator>
PseudoRandomNumberGenerator::Instance(void) {
  static auto instance = std::make_shared<PseudoRandomNumberGenerator>();

  return instance;
}

Seed PseudoRandomNumberGenerator::get_seed(void) const { return seed; }

void PseudoRandomNumberGenerator::set_seed(const Seed seed) {
  this->seed = seed;

  for (auto &[key, value] : distribution) {
    value->set_seed(seed);
  }
}

void PseudoRandomNumberGenerator::set_exponential(const std::string key,
                                                  const double mean) {
  distribution[key] = std::make_unique<Exponential>(seed, mean);
}

void PseudoRandomNumberGenerator::set_poisson(const std::string key,
                                              const double mean) {
  distribution[key] = std::make_unique<Poisson>(seed, mean);
}

void PseudoRandomNumberGenerator::set_normal(const std::string key,
                                             const double mean,
                                             const double deviation) {
  distribution[key] = std::make_unique<Normal>(seed, mean, deviation);
}

void PseudoRandomNumberGenerator::set_discrete(
    const std::string key, const std::vector<double> &list) {
  distribution[key] = std::make_unique<Discrete>(seed, list);
}

void PseudoRandomNumberGenerator::set_uniform(const std::string key,
                                              const double min,
                                              const double max) {
  distribution[key] = std::make_unique<Uniform>(seed, min, max);
}

double PseudoRandomNumberGenerator::next(const std::string key) {
  return distribution.at(key)->next();
}
