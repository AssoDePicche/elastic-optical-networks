#include "configuration.h"

#include <filesystem>
#include <fstream>
#include <sstream>

#include "file_system.h"

#include <nlohmann/json.hpp>

namespace core {
std::shared_ptr<Configuration> Configuration::From(const std::string &filename) {
  if (!FileSystem::Exists(filename)) {
    return nullptr;
  }

  std::ifstream stream(filename);

  nlohmann::json json = nlohmann::json::parse(stream);

static const std::unordered_map<std::string, SpectrumAllocator>
      spectrumAllocationStrategies{{"best-fit", BestFit},
                                   {"first-fit", FirstFit},
                                   {"last-fit", LastFit},
                                   {"random-fit", RandomFit},
                                   {"worst-fit", WorstFit}};

  static const std::unordered_map<std::string,
                                  ModulationStrategyFactory::Option>
      modulationOptions{
          {"passband", ModulationStrategyFactory::Option::Passband},
          {"gigabits", ModulationStrategyFactory::Option::Gigabits},
          {"terabits", ModulationStrategyFactory::Option::Terabits},
      };


  std::shared_ptr<Configuration> configuration = std::make_shared<Configuration>();

  configuration->enableLogging = static_cast<bool>(json["enable-logging"]);

  configuration->logger = std::make_shared<Logger>(configuration->enableLogging);

  configuration->exportDataset = static_cast<bool>(json["export-dataset"]);

  configuration->agent = json["params"]["agent"];

  configuration->ignoreFirst = static_cast<bool>(json["params"]["ignore-first"]);

  configuration->samplingTime = static_cast<uint64_t>(json["params"]["sampling-time"]);

  configuration->timeUnits = static_cast<double>(json["params"]["simulation-duration"]);

  configuration->arrivalRate = static_cast<double>(json["params"]["arrival-rate"]);

  configuration->serviceRate = static_cast<double>(json["params"]["service-rate"]);

  configuration->iterations = static_cast<double>(json["params"]["iterations"]);

  configuration->spectrumWidth = static_cast<double>(json["params"]["spectrum-width"]);

  configuration->slotWidth = static_cast<double>(json["params"]["slot-width"]);

  configuration->FSUsPerLink = configuration->spectrumWidth / configuration->slotWidth;

  configuration->modulationOption = modulationOptions.at(json["params"]["modulation"]);

  const std::vector<nlohmann::json> requests = json["params"]["requests"];

  for (const auto &row : requests) {
    RequestType requestType;

    requestType.type = row["type"];

    requestType.modulation = row["modulation"];

    requestType.bandwidth = row["bandwidth"];

    requestType.allocator = spectrumAllocationStrategies.at(row["allocator"]);

    requestType.blocking = 0;

    requestType.FSUs = 0;

    requestType.counting = 0;

    configuration->requestTypes[requestType.type] = requestType;
  }

  const std::vector<nlohmann::json> modulations = json["modulation"];

  for (const auto &row : modulations) {
    configuration->modulations[row["type"]] = row["bits-per-symbol"];
  }

  for (auto &request : configuration->requestTypes) {
    const ModulationStrategyFactory factory;

    const auto spectralEfficiency =
        configuration->modulations.at(request.second.modulation);

    const auto strategy =
        factory.From(configuration->modulationOption, configuration->slotWidth,
                     spectralEfficiency);

    request.second.FSUs = configuration->modulationOption ==
                                  ModulationStrategyFactory::Option::Passband
                              ? strategy->compute(request.second.bandwidth)
                              : strategy->compute(graph::Cost::max().value);

    request.second.counting = 0u;

    request.second.blocking = 0u;
  }

  configuration->minFSUsPerRequest =
      (*configuration->requestTypes.begin()).second.FSUs;

  for (const auto& request : configuration->requestTypes) {
    if (request.second.FSUs < configuration->minFSUsPerRequest) {
      configuration->minFSUsPerRequest = request.second.FSUs;
    }
  }

  configuration->fragmentationStrategies = {
      {"absolute_fragmentation", std::make_shared<AbsoluteFragmentation>()},
      {"external_fragmentation", std::make_shared<ExternalFragmentation>()},
      {"entropy_based_fragmentation",
       std::make_shared<EntropyBasedFragmentation>(
           configuration->minFSUsPerRequest)},
  };

  configuration->probs = {};

  for (const auto& row : requests) {
    configuration->probs.push_back(row["ratio"]);
  }

  const auto graph = graph::Graph::from(json["params"]["topology"]);

  if (!graph.has_value()) {
    return nullptr;
  }

  configuration->graph = std::move(graph.value());

  return configuration;
}
}  // namespace core
