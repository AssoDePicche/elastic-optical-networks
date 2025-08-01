#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "graph.h"
#include "json.h"
#include "logger.h"
#include "prng.h"
#include "request.h"
#include "spectrum.h"

struct Configuration final {
  Graph graph;
  ModulationStrategyFactory::Option modulationOption;
  std::unordered_map<std::string, FragmentationStrategy>
      fragmentationStrategies;
  std::unordered_map<std::string, RequestType> requestTypes;
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
  std::shared_ptr<Logger> logger;

  [[nodiscard]] static std::optional<std::shared_ptr<Configuration>> From(
      const Json &);
};
