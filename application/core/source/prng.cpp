#include "prng.h"

Distribution::~Distribution() {}

Exponential::Exponential(const double mean) : _distribution{mean} {}

double Exponential::next(std::mt19937& generator) {
  return _distribution(generator);
}

Poisson::Poisson(const double mean) : _distribution{mean} {}

double Poisson::next(std::mt19937& generator) {
  return static_cast<double>(_distribution(generator));
}

Normal::Normal(const double mean, const double deviation)
    : _distribution{mean, deviation} {}

double Normal::next(std::mt19937& generator) {
  return _distribution(generator);
}

double Discrete::next(std::mt19937& generator) {
  return static_cast<double>(_distribution(generator));
}

Uniform::Uniform(const double min, const double max)
    : _distribution{min, max} {}

double Uniform::next(std::mt19937& generator) {
  return _distribution(generator);
}

std::shared_ptr<PseudoRandomNumberGenerator>
PseudoRandomNumberGenerator::Instance(void) {
  static auto instance = std::make_shared<PseudoRandomNumberGenerator>();

  return instance;
}

uint64_t PseudoRandomNumberGenerator::get_seed(void) const { return seed; }

void PseudoRandomNumberGenerator::set_seed(const uint64_t seed) {
  this->generator.seed(seed);

  this->seed = seed;
}

void PseudoRandomNumberGenerator::set_random_seed(void) {
  set_seed(random_device());
}

void PseudoRandomNumberGenerator::set_exponential(const std::string key,
                                                  const double mean) {
  _distribution[key] = std::make_unique<Exponential>(mean);
}

void PseudoRandomNumberGenerator::set_poisson(const std::string key,
                                              const double mean) {
  _distribution[key] = std::make_unique<Poisson>(mean);
}

void PseudoRandomNumberGenerator::set_normal(const std::string key,
                                             const double mean,
                                             const double deviation) {
  _distribution[key] = std::make_unique<Normal>(mean, deviation);
}

void PseudoRandomNumberGenerator::set_uniform(const std::string key,
                                              const double min,
                                              const double max) {
  _distribution[key] = std::make_unique<Uniform>(min, max);
}

double PseudoRandomNumberGenerator::next(const std::string key) {
  return _distribution.at(key)->next(this->generator);
}
