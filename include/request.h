#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "graph.h"
#include "spectrum.h"

struct Request {
  route_t route;
  slice_t slice;
  unsigned bandwidth{};

  Request(void) = default;

  Request(const route_t &, const unsigned);
};

[[nodiscard]] auto from_gigabits_transmission(const double) -> unsigned;

[[nodiscard]] auto from_terabits_transmission(const double) -> unsigned;

[[nodiscard]] auto make_key(unsigned, unsigned) -> unsigned;

[[nodiscard]] auto route_keys(const route_t &) -> std::vector<unsigned>;

[[nodiscard]] auto make_hashmap(const Graph &,
                                const unsigned) -> std::map<unsigned, Spectrum>;

[[nodiscard]] auto dispatch_request(Request &, std::map<unsigned, Spectrum> &,
                                    const SpectrumAllocator &) -> bool;
