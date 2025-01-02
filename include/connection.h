#pragma once

#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "graph.h"
#include "spectrum.h"

struct Connection {
  Path path;
  Slice slice;
  unsigned slots{};

  Connection(void) = default;

  Connection(const Path &, const unsigned);
};

[[nodiscard]] auto from_gigabits_transmission(const double) -> unsigned;

[[nodiscard]] auto from_terabits_transmission(const double) -> unsigned;

[[nodiscard]] auto make_key(unsigned, unsigned) -> unsigned;

[[nodiscard]] auto path_keys(const Path &) -> std::vector<unsigned>;

[[nodiscard]] auto make_hashmap(const Graph &,
                                const unsigned) -> std::map<unsigned, Spectrum>;

[[nodiscard]] auto make_connection(Connection &, std::map<unsigned, Spectrum> &,
                                   const SpectrumAllocator &) -> bool;
