#include "request.h"

#include <array>
#include <cassert>
#include <ranges>
#include <utility>

namespace core {
Request::Request(const graph::Route& route) : route{route} {}

PassbandModulation::PassbandModulation(double slotWidth,
                                       uint64_t spectralEfficiency)
    : slotWidth{slotWidth}, spectralEfficiency{spectralEfficiency} {}

uint64_t PassbandModulation::compute(const double bandwidth) const {
  return bandwidth / (spectralEfficiency * slotWidth);
}

uint64_t GigabitsTransmission::compute(const double distance) const {
  constexpr std::array<std::pair<double, uint64_t>, 7> thresholds = {{
      {160.0, 5u},
      {880.0, 6u},
      {2480.0, 7u},
      {3120.0, 9u},
      {5000.0, 10u},
      {6080.0, 12u},
      {8000.0, 13u},
  }};

  for (const auto& [threshold, FSUs] : thresholds) {
    if (distance <= threshold) {
      return FSUs;
    }
  }

  return FSU::max;
}

uint64_t TerabitsTransmission::compute(const double distance) const {
  constexpr std::array<std::pair<double, uint64_t>, 7> thresholds = {{
      {400.0, 14u},
      {800.0, 15u},
      {1600.0, 17u},
      {3040.0, 19u},
      {4160.0, 22u},
      {6400.0, 25u},
      {8000.0, 28u},
  }};

  for (const auto& [threshold, FSUs] : thresholds) {
    if (distance <= threshold) {
      return FSUs;
    }
  }

  return FSU::max;
}

ModulationStrategy ModulationStrategyFactory::From(
    Option option, double slotWidth, uint64_t spectralEfficiency) const {
  switch (option) {
    case Option::Passband:
      return std::make_shared<PassbandModulation>(slotWidth,
                                                  spectralEfficiency);
    case Option::Gigabits:
      return std::make_shared<GigabitsTransmission>();
    case Option::Terabits:
      return std::make_shared<TerabitsTransmission>();
  }

  return nullptr;
}
}  // namespace core
