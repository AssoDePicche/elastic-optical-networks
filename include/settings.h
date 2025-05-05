#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "distribution.h"
#include "graph.h"
#include "spectrum.h"

struct RequestType {
  std::string type;
  std::string modulation;
  SpectrumAllocator allocator;
  double bandwidth;
};

struct Settings {
  Graph graph;
  SpectrumAllocator spectrum_allocator;
  std::unordered_map<std::string, RequestType> requests;
  Seed seed;
  double arrival_rate;
  double service_rate;
  double time_units;
  unsigned bandwidth;
  double spectrum_width;
  double slot_width;

  [[nodiscard]] static auto from(const std::vector<std::string> &)
      -> std::optional<Settings>;

  [[nodiscard]] static auto from(const std::string &)
      -> std::optional<Settings>;
};
