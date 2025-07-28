#pragma once

#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "graph.h"
#include "json.h"
#include "prng.h"
#include "request.h"
#include "spectrum.h"

struct Configuration final {
  Graph graph;
  KeyGenerator keyGenerator;
  ModulationOption modulationOption;
  std::unordered_map<std::string, FragmentationStrategy>
      fragmentationStrategies;
  std::unordered_map<std::string, RequestType> requests;
  std::unordered_map<std::string, unsigned> modulations;
  std::vector<double> probs;
  double arrivalRate;
  double serviceRate;
  double timeUnits;
  double spectrumWidth;
  double slotWidth;
  unsigned FSUsPerLink;
  unsigned minFSUsPerRequest;
  unsigned iterations;
  unsigned samplingTime;
  bool ignoreFirst;
  bool exportDataset;
  bool enableLogging;

  [[nodiscard]] static std::optional<Configuration> From(const Json &);
};
