#pragma once

#include <memory>
#include <string>

#include "route.h"
#include "spectrum.h"

struct RequestType final {
  std::string type;
  std::string modulation;
  SpectrumAllocator allocator;
  double bandwidth;
  uint64_t blocking;
  uint64_t FSUs;
  uint64_t counting;
};

struct Request final {
  RequestType type;
  Route route;
  Slice slice;
  bool accepted;

  Request(void) = default;

  Request(const Route &);
};

struct Modulation {
  virtual ~Modulation() = default;

  [[nodiscard]] virtual uint64_t compute(const double) const = 0;
};

class PassbandModulation : public Modulation {
  double slotWidth;
  uint64_t spectralEfficiency;

 public:
  PassbandModulation(double, uint64_t);

  [[nodiscard]] uint64_t compute(const double) const override;
};

struct DistanceAdaptativeModulation : public Modulation {};

class GigabitsTransmission : public DistanceAdaptativeModulation {
  [[nodiscard]] uint64_t compute(const double) const override;
};

class TerabitsTransmission : public DistanceAdaptativeModulation {
  [[nodiscard]] uint64_t compute(const double) const override;
};

using ModulationStrategy = std::shared_ptr<Modulation>;

class ModulationStrategyFactory final {
 public:
  enum class Option {
    Passband,
    Gigabits,
    Terabits,
  };

  [[nodiscard]] ModulationStrategy From(Option, double, uint64_t) const;
};
