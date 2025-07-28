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
  unsigned blocking;
  unsigned FSUs;
  unsigned counting;
};

using PairingFunction = std::function<unsigned(unsigned, unsigned)>;

[[nodiscard]] unsigned CantorPairingFunction(unsigned, unsigned);

enum class ModulationOption {
  Passband,
  Gigabits,
  Terabits,
};

struct Modulation {
  virtual ~Modulation() = default;

  [[nodiscard]] virtual unsigned compute(const double) const = 0;
};

class PassbandModulation : public Modulation {
 public:
  PassbandModulation(double, unsigned);

  [[nodiscard]] unsigned compute(const double) const override;

 private:
  double slotWidth;
  unsigned spectralEfficiency;
};

struct DistanceAdaptativeModulation : public Modulation {};

class GigabitsTransmission : public DistanceAdaptativeModulation {
  [[nodiscard]] unsigned compute(const double) const override;
};

class TerabitsTransmission : public DistanceAdaptativeModulation {
  [[nodiscard]] unsigned compute(const double) const override;
};

using ModulationStrategy = std::shared_ptr<Modulation>;

class ModulationStrategyFactory final {
 public:
  ModulationStrategyFactory(ModulationOption);

  [[nodiscard]] ModulationStrategy From(double, unsigned) const;

 private:
  ModulationOption option;
};

struct Request {
  Route route;
  Slice slice;
  unsigned FSUs{};
  bool accepted;

  Request(void) = default;

  Request(const Route &, const unsigned);
};

class KeyGenerator final {
 public:
  KeyGenerator(void) = default;

  KeyGenerator(PairingFunction);

  [[nodiscard]] unsigned generate(const Vertex, const Vertex) const;

  [[nodiscard]] std::unordered_set<unsigned> generate(const Route &) const;

 private:
  PairingFunction function;
};

using Carriers = std::unordered_map<unsigned, Spectrum>;

class Dispatcher final {
 public:
  Dispatcher(Graph, KeyGenerator, unsigned);

  [[nodiscard]] bool dispatch(Request &, const SpectrumAllocator &);

  [[nodiscard]] Carriers GetCarriers(void) const;

  void release(Request &);

 private:
  KeyGenerator keyGenerator;
  Carriers carriers;
  unsigned FSUsPerLink;
};
