#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "graph.h"
#include "math.h"
#include "route.h"
#include "spectrum.h"

struct Request {
  Route route;
  Slice slice;
  unsigned FSUs{};
  bool accepted;

  Request(void) = default;

  Request(const Route &, const unsigned);
};

[[nodiscard]] unsigned from_modulation(double, unsigned, double);

[[nodiscard]] unsigned from_gigabits_transmission(const double);

[[nodiscard]] unsigned from_terabits_transmission(const double);

[[nodiscard]] std::unordered_set<unsigned> route_keys(const Route &,
                                                      PairingFunction);

using Hashmap = std::unordered_map<unsigned, Spectrum>;

[[nodiscard]] Hashmap GetHashmap(const Graph &, const unsigned,
                                 PairingFunction);

[[nodiscard]] bool Dispatch(Request &, Hashmap &, const SpectrumAllocator &,
                            PairingFunction);
