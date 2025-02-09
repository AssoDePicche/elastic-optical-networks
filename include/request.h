#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "graph.h"
#include "spectrum.h"

using bandwidth_t = unsigned;

constexpr auto max_bandwidth = std::numeric_limits<bandwidth_t>::max();

struct Request {
  route_t route;
  slice_t slice;
  bandwidth_t bandwidth{};

  Request(void) = default;

  Request(const route_t &, const bandwidth_t);
};

[[nodiscard]] bandwidth_t from_gigabits_transmission(const cost_t) noexcept;

[[nodiscard]] bandwidth_t from_terabits_transmission(const cost_t) noexcept;

[[nodiscard]] auto make_key(unsigned, unsigned) -> unsigned;

[[nodiscard]] auto route_keys(const route_t &) -> std::vector<unsigned>;

[[nodiscard]] auto make_hashmap(const Graph &,
                                const unsigned) -> std::map<unsigned, Spectrum>;

[[nodiscard]] auto dispatch_request(Request &, std::map<unsigned, Spectrum> &,
                                    const SpectrumAllocator &) -> bool;
