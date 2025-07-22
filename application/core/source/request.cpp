#include "request.h"

#include <array>
#include <cassert>
#include <ranges>
#include <utility>

Request::Request(const Route &route, const unsigned FSUs)
    : route{route}, FSUs{FSUs} {}

PassbandModulation::PassbandModulation(double slotWidth,
                                       unsigned spectralEfficiency)
    : slotWidth{slotWidth}, spectralEfficiency{spectralEfficiency} {}

unsigned PassbandModulation::compute(const double bandwidth) const {
  return bandwidth / (spectralEfficiency * slotWidth);
}

unsigned GigabitsTransmission::compute(const double distance) const {
  constexpr std::array<std::pair<double, unsigned>, 7> thresholds = {{
      {160.0, 5u},
      {880.0, 6u},
      {2480.0, 7u},
      {3120.0, 9u},
      {5000.0, 10u},
      {6080.0, 12u},
      {8000.0, 13u},
  }};

  for (const auto &[threshold, FSUs] : thresholds) {
    if (distance <= threshold) {
      return FSUs;
    }
  }

  return FSU::max;
}

unsigned TerabitsTransmission::compute(const double distance) const {
  constexpr std::array<std::pair<double, unsigned>, 7> thresholds = {{
      {400.0, 14u},
      {800.0, 15u},
      {1600.0, 17u},
      {3040.0, 19u},
      {4160.0, 22u},
      {6400.0, 25u},
      {8000.0, 28u},
  }};

  for (const auto &[threshold, FSUs] : thresholds) {
    if (distance <= threshold) {
      return FSUs;
    }
  }

  return FSU::max;
}

ModulationStrategyFactory::ModulationStrategyFactory(ModulationOption option)
    : option{option} {}

ModulationStrategy ModulationStrategyFactory::From(
    double slotWidth, unsigned spectralEfficiency) const {
  switch (option) {
    case ModulationOption::Passband:
      return std::make_shared<PassbandModulation>(slotWidth,
                                                  spectralEfficiency);
    case ModulationOption::Gigabits:
      return std::make_shared<GigabitsTransmission>();
    case ModulationOption::Terabits:
      return std::make_shared<TerabitsTransmission>();
  }

  return nullptr;
}

KeyGenerator::KeyGenerator(PairingFunction function) : function{function} {}

unsigned KeyGenerator::generate(const Vertex source,
                                const Vertex destination) const {
  return function(source, destination);
}

std::unordered_set<unsigned> KeyGenerator::generate(const Route &route) const {
  const auto &[vertices, cost] = route;

  assert(!vertices.empty());

  std::unordered_set<unsigned> keys;

  for (const auto &index : std::views::iota(1u, vertices.size())) {
    const auto x = *std::next(vertices.begin(), index - 1);

    const auto y = *std::next(vertices.begin(), index);

    keys.insert(function(x, y));
  }

  return keys;
}

Dispatcher::Dispatcher(Graph graph, KeyGenerator keyGenerator,
                       unsigned FSUsPerLink)
    : keyGenerator{keyGenerator}, FSUsPerLink{FSUsPerLink} {
  for (const auto &[source, destination, cost] : graph.get_edges()) {
    const auto key = keyGenerator.generate(source, destination);

    carriers[key] = Spectrum(FSUsPerLink);
  }
}

bool Dispatcher::dispatch(Request &request,
                          const SpectrumAllocator &allocator) {
  assert(request.FSUs <= spectrum.size());

  const auto keys = keyGenerator.generate(request.route);

  const auto first = *keys.begin();

  const auto slice = allocator(carriers[first], request.FSUs);

  if (!slice.has_value()) {
    return false;
  }

  request.slice = slice.value();

  for (const auto &key : keys) {
    if (carriers[key].available() < request.FSUs ||
        !carriers[key].available_at(request.slice)) {
      return false;
    }
  }

  const auto allocate = [&](const auto key) {
    carriers[key].allocate(request.slice);
  };

  std::for_each(keys.begin(), keys.end(), allocate);

  return true;
}

Carriers Dispatcher::GetCarriers(void) const { return carriers; }

void Dispatcher::release(Request &request) {
  const auto keys = keyGenerator.generate(request.route);

  std::for_each(keys.begin(), keys.end(), [&](const auto key) {
    assert(!carriers[key].available_at(slice));

    carriers[key].deallocate(request.slice);
  });
}
