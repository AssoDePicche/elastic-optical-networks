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
  SpectrumAllocator spectrumAllocator;
  std::unordered_map<std::string, RequestType> requests;
  Seed seed;
  double arrivalRate;
  double serviceRate;
  double timeUnits;
  unsigned bandwidth;
  double spectrumWidth;
  double slotWidth;
  bool ignoreFirst;
  bool exportDataset;
  bool enableLogging;

  [[nodiscard]] static auto from(const std::vector<std::string> &)
      -> std::optional<Settings>;

  [[nodiscard]] static auto from(const std::string &)
      -> std::optional<Settings>;
};
