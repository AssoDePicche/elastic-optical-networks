#pragma once

#include <initializer_list>
#include <memory>
#include <random>
#include <string>
#include <unordered_map>
#include <vector>

using Seed = long long unsigned;

class Distribution {
 public:
  Distribution(const Seed);

  [[nodiscard]] virtual double next(void) = 0;

  [[nodiscard]] Seed get_seed(void) const;

  void set_seed(const Seed);

 private:
  Seed seed;

 protected:
  std::mt19937 generator;
};

class Exponential final : public Distribution {
 public:
  Exponential(const Seed, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::exponential_distribution<double> distribution;
};

class Poisson final : public Distribution {
 public:
  Poisson(const Seed, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::poisson_distribution<int> distribution;
};

class Normal final : public Distribution {
 public:
  Normal(const Seed, const double, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::normal_distribution<double> distribution;
};

class Discrete final : public Distribution {
 public:
  Discrete(const Seed, const std::vector<double> &);

  [[nodiscard]] double next(void) override;

 private:
  std::discrete_distribution<int> distribution;
};

class Uniform final : public Distribution {
 public:
  Uniform(const Seed, const double, const double);

  [[nodiscard]] double next(void) override;

 private:
  std::uniform_real_distribution<double> distribution;
};

class PseudoRandomNumberGenerator final {
 public:
  static std::shared_ptr<PseudoRandomNumberGenerator> Instance(void);

  [[nodiscard]] Seed get_seed(void) const;

  void set_seed(const Seed);

  void set_exponential(const std::string, const double);

  void set_poisson(const std::string, const double);

  void set_normal(const std::string, const double, const double);

  void set_discrete(const std::string, const std::vector<double> &);

  void set_uniform(const std::string, const double, const double);

  [[nodiscard]] double next(const std::string);

 private:
  std::unordered_map<std::string, std::shared_ptr<Distribution>> distribution;
  Seed seed;
};
