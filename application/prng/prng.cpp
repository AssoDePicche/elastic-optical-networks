#include "prng.h"

#include "exponential.h"
#include "normal.h"
#include "poisson.h"
#include "uniform.h"

namespace prng {
std::shared_ptr<PseudoRandomNumberGenerator>
PseudoRandomNumberGenerator::Instance(void) {
  static auto instance = std::make_shared<PseudoRandomNumberGenerator>();

  return instance;
}

uint64_t PseudoRandomNumberGenerator::GetSeed(void) const { return _seed; }

void PseudoRandomNumberGenerator::SetSeed(const uint64_t seed) {
  this->_generator.seed(seed);

  this->_seed = seed;
}

void PseudoRandomNumberGenerator::SetRandomSeed(void) {
  SetSeed(_random_device());
}

void PseudoRandomNumberGenerator::SetExponentialVariable(const std::string key,
                                                         const double mean) {
  _distribution[key] = std::make_unique<Exponential>(mean);
}

void PseudoRandomNumberGenerator::SetPoissonVariable(const std::string key,
                                                     const double mean) {
  _distribution[key] = std::make_unique<Poisson>(mean);
}

void PseudoRandomNumberGenerator::SetNormalVariable(const std::string key,
                                                    const double mean,
                                                    const double deviation) {
  _distribution[key] = std::make_unique<Normal>(mean, deviation);
}

void PseudoRandomNumberGenerator::SetUniformVariable(const std::string key,
                                                     const double min,
                                                     const double max) {
  _distribution[key] = std::make_unique<Uniform>(min, max);
}

double PseudoRandomNumberGenerator::Next(const std::string key) {
  return _distribution.at(key)->Next(_generator);
}
}  // namespace prng
