#include "request.h"

#include <array>
#include <cassert>
#include <ranges>
#include <utility>

Request::Request(const route_t &route, const unsigned FSUs)
    : route{route}, FSUs{FSUs} {}

auto from_modulation(double bandwidth, unsigned spectralEfficiency,
                     double slotWidth) -> unsigned {
  return bandwidth / (spectralEfficiency * slotWidth);
}

auto from_gigabits_transmission(const double distance) -> unsigned {
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

auto from_terabits_transmission(const double distance) -> unsigned {
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

std::unordered_set<unsigned> route_keys(const route_t &route,
                                        PairingFunction make_key) {
  const auto &[vertices, cost] = route;

  assert(!vertices.empty());

  std::unordered_set<unsigned> keys;

  for (const auto &index : std::views::iota(1u, vertices.size())) {
    const auto x = *std::next(vertices.begin(), index - 1);

    const auto y = *std::next(vertices.begin(), index);

    keys.insert(make_key(x, y));
  }

  return keys;
}

Hashmap GetHashmap(const Graph &graph, const unsigned FSUsPerLink,
                   PairingFunction make_key) {
  Hashmap hashmap;

  const auto edges = graph.get_edges();

  for (const auto &[source, destination, cost] : edges) {
    const auto key = make_key(source, destination);

    hashmap[key] = Spectrum(FSUsPerLink);
  }

  return hashmap;
}

bool Dispatch(Request &request, Hashmap &hashmap,
              const SpectrumAllocator &spectrum_allocator,
              PairingFunction make_key) {
  const auto keys{route_keys(request.route, make_key)};

  const auto first = *keys.begin();

  const auto slice{spectrum_allocator(hashmap[first], request.FSUs)};

  if (!slice.has_value()) {
    return false;
  }

  request.slice = slice.value();

  for (const auto &key : keys) {
    if (!hashmap[key].available_at(request.slice)) {
      return false;
    }
  }

  std::for_each(keys.begin(), keys.end(),
                [&](const auto key) { hashmap[key].allocate(request.slice); });

  return true;
}
