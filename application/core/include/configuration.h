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
  ModulationStrategyFactory::Option modulationOption;
  std::unordered_map<std::string, FragmentationStrategy>
      fragmentationStrategies;
  std::unordered_map<std::string, RequestType> requests;
  std::unordered_map<std::string, uint64_t> modulations;
  std::vector<double> probs;
  double arrivalRate;
  double serviceRate;
  double timeUnits;
  double spectrumWidth;
  double slotWidth;
  uint64_t FSUsPerLink;
  uint64_t minFSUsPerRequest;
  uint64_t iterations;
  uint64_t samplingTime;
  bool ignoreFirst;
  bool exportDataset;
  bool enableLogging;

  [[nodiscard]] static std::optional<Configuration> From(const Json &);
};
