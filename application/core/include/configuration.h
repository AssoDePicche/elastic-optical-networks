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
  std::unordered_map<std::string, FragmentationStrategy>
      fragmentationStrategies;
  std::unordered_map<std::string, RequestType> requests;
  std::unordered_map<std::string, unsigned> modulations;
  std::vector<double> probs;
  KeyGenerator keyGenerator;
  ModulationOption modulationOption;
  double arrivalRate;
  double serviceRate;
  double timeUnits;
  unsigned FSUsPerLink;
  double spectrumWidth;
  double slotWidth;
  bool ignoreFirst;
  bool exportDataset;
  bool enableLogging;
  unsigned minFSUsPerRequest;
  unsigned iterations;
  unsigned samplingTime;

  [[nodiscard]] static std::optional<Configuration> From(const Json &);
};
