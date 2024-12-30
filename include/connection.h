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
  std::size_t slots{};

  Connection(void) = default;

  Connection(const Path &, const std::size_t);
};

[[nodiscard]] auto from_gigabits_transmission(const double) -> std::size_t;

[[nodiscard]] auto from_terabits_transmission(const double) -> std::size_t;

[[nodiscard]] auto make_key(std::size_t, std::size_t) -> std::size_t;

[[nodiscard]] auto path_keys(const Path &) -> std::vector<std::size_t>;

[[nodiscard]] auto make_hashmap(const Graph &, const std::size_t)
    -> std::map<std::size_t, Spectrum>;

[[nodiscard]] auto make_connection(Connection &,
                                   std::map<std::size_t, Spectrum> &,
                                   const SpectrumAllocator &) -> bool;
