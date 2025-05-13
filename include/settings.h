#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "distribution.h"
#include "graph.h"
#include "json.h"
#include "spectrum.h"

struct RequestType {
  std::string type;
  std::string modulation;
  SpectrumAllocator allocator;
  double bandwidth;
  unsigned blocking;
  unsigned resources;
  unsigned counting;
};

struct Settings {
  Graph graph;
  SpectrumAllocator spectrumAllocator;
  std::unordered_map<std::string, RequestType> requests;
  std::unordered_map<std::string, unsigned> modulations;
  std::vector<double> probs;
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

  [[nodiscard]] static auto From(const Json &) -> std::optional<Settings>;
};
