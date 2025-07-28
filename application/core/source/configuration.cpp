
#include "configuration.h"

std::optional<Configuration> Configuration::From(const Json &json) {
  static const std::unordered_map<std::string, SpectrumAllocator>
      spectrumAllocationStrategies{{"best-fit", BestFit},
                                   {"first-fit", FirstFit},
                                   {"last-fit", LastFit},
                                   {"random-fit", RandomFit},
                                   {"worst-fit", WorstFit}};

  static const std::unordered_map<std::string, PairingFunction>
      pairingFunctionStrategies{
          {"cantor", CantorPairingFunction},
      };

  static const std::unordered_map<std::string, ModulationOption>
      modulationOptions{
          {"passband", ModulationOption::Passband},
          {"gigabits", ModulationOption::Gigabits},
          {"terabits", ModulationOption::Terabits},
      };

  Configuration configuration;

  configuration.enableLogging = json.Get<bool>("enable-logging").value();

  configuration.exportDataset = json.Get<bool>("export-dataset").value();

  configuration.ignoreFirst = json.Get<bool>("params.ignore-first").value();

  configuration.samplingTime =
      json.Get<unsigned>("params.sampling-time").value();

  configuration.timeUnits =
      json.Get<double>("params.simulation-duration").value();

  configuration.arrivalRate = json.Get<double>("params.arrival-rate").value();

  configuration.serviceRate = json.Get<double>("params.service-rate").value();

  configuration.iterations = json.Get<unsigned>("params.iterations").value();

  configuration.spectrumWidth =
      json.Get<double>("params.spectrum-width").value();

  configuration.slotWidth = json.Get<double>("params.slot-width").value();

  configuration.FSUsPerLink =
      configuration.spectrumWidth / configuration.slotWidth;

  configuration.keyGenerator = KeyGenerator(pairingFunctionStrategies.at(
      json.Get<std::string>("params.pairing-function").value_or("cantor")));

  configuration.modulationOption = modulationOptions.at(
      json.Get<std::string>("params.modulation").value_or("passband"));

  const auto requests =
      json.Get<std::vector<nlohmann::json>>("params.requests");

  for (const auto &row : requests.value()) {
    RequestType request;

    request.type = row["type"];

    request.modulation = row["modulation"];

    request.bandwidth = row["bandwidth"];

    request.allocator = spectrumAllocationStrategies.at(row["allocator"]);

    request.blocking = 0u;

    request.FSUs = 0u;

    request.counting = 0u;

    configuration.requests[request.type] = request;
  }

  const auto modulations = json.Get<std::vector<nlohmann::json>>("modulation");

  for (const auto &row : modulations.value()) {
    configuration.modulations[row["type"]] = row["bits-per-symbol"];
  }

  for (auto &request : configuration.requests) {
    const ModulationStrategyFactory factory(configuration.modulationOption);

    const auto spectralEfficiency =
        configuration.modulations.at(request.second.modulation);

    const auto strategy =
        factory.From(configuration.slotWidth, spectralEfficiency);

    request.second.FSUs =
        configuration.modulationOption == ModulationOption::Passband
            ? strategy->compute(request.second.bandwidth)
            : strategy->compute(Cost::max().value);

    request.second.counting = 0u;

    request.second.blocking = 0u;
  }

  configuration.minFSUsPerRequest =
      (*configuration.requests.begin()).second.FSUs;

  for (const auto &request : configuration.requests) {
    if (request.second.FSUs < configuration.minFSUsPerRequest) {
      configuration.minFSUsPerRequest = request.second.FSUs;
    }
  }

  configuration.fragmentationStrategies = {
      {"absolute_fragmentation", std::make_shared<AbsoluteFragmentation>()},
      {"external_fragmentation", std::make_shared<ExternalFragmentation>()},
      {"entropy_based_fragmentation",
       std::make_shared<EntropyBasedFragmentation>(
           configuration.minFSUsPerRequest)},
  };

  configuration.probs = {};

  for (const auto &row : requests.value()) {
    configuration.probs.push_back(row["ratio"]);
  }

  const auto graph =
      Graph::from(json.Get<std::string>("params.topology").value());

  if (!graph.has_value()) {
    return std::nullopt;
  }

  configuration.graph = graph.value();

  return configuration;
}
