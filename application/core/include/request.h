#pragma once

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph.h"
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
  uint64_t FSUs{};
  bool accepted;

  Request(void) = default;

  Request(const Route &, const uint64_t);
};

using PairingFunction = std::function<uint64_t(uint64_t, uint64_t)>;

[[nodiscard]] uint64_t CantorPairingFunction(uint64_t, uint64_t);

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

class KeyGenerator final {
  PairingFunction function;

 public:
  KeyGenerator(void) = default;

  KeyGenerator(PairingFunction);

  [[nodiscard]] uint64_t generate(const Vertex, const Vertex) const;

  [[nodiscard]] std::unordered_set<uint64_t> generate(const Route &) const;
};

using Carriers = std::unordered_map<uint64_t, Spectrum>;

class Dispatcher final {
  KeyGenerator keyGenerator;
  Carriers carriers;
  uint64_t FSUsPerLink;

 public:
  Dispatcher(Graph, KeyGenerator, uint64_t);

  [[nodiscard]] bool dispatch(Request &, const SpectrumAllocator &);

  [[nodiscard]] Carriers GetCarriers(void) const;

  void release(Request &);
};
