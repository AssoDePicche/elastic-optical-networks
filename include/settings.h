#pragma once

#include <optional>
#include <string>
#include <vector>

#include "distribution.h"
#include "graph.h"
#include "spectrum.h"

struct Settings {
  Graph graph;
  SpectrumAllocator spectrum_allocator;
  Seed seed;
  double arrival_rate;
  double service_rate;
  double time_units;
  std::size_t channels;

  [[nodiscard]] static auto from(const std::vector<std::string> &)
      -> std::optional<Settings>;
};
