#include "settings.h"

#include <algorithm>

#include "request.h"

std::optional<Settings> Settings::From(const Json &json) {
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

  Settings settings;

  settings.enableLogging = json.Get<bool>("enable-logging").value();

  settings.exportDataset = json.Get<bool>("export-dataset").value();

  settings.ignoreFirst = json.Get<bool>("params.ignore-first").value();

  settings.samplingTime = json.Get<unsigned>("params.sampling-time").value();

  settings.timeUnits = json.Get<double>("params.simulation-duration").value();

  settings.arrivalRate = json.Get<double>("params.arrival-rate").value();

  settings.serviceRate = json.Get<double>("params.service-rate").value();

  settings.iterations = json.Get<unsigned>("params.iterations").value();

  settings.spectrumWidth = json.Get<double>("params.spectrum-width").value();

  settings.slotWidth = json.Get<double>("params.slot-width").value();

  settings.FSUsPerLink = settings.spectrumWidth / settings.slotWidth;

  settings.keyGenerator = KeyGenerator(pairingFunctionStrategies.at(
      json.Get<std::string>("params.pairing-function").value_or("cantor")));

  settings.modulationOption = modulationOptions.at(
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

    settings.requests[request.type] = request;
  }

  const auto modulations = json.Get<std::vector<nlohmann::json>>("modulation");

  for (const auto &row : modulations.value()) {
    settings.modulations[row["type"]] = row["bits-per-symbol"];
  }

  for (auto &request : settings.requests) {
    const ModulationStrategyFactory factory(settings.modulationOption);

    const auto spectralEfficiency =
        settings.modulations.at(request.second.modulation);

    const auto strategy = factory.From(settings.slotWidth, spectralEfficiency);

    request.second.FSUs =
        settings.modulationOption == ModulationOption::Passband
            ? strategy->compute(request.second.bandwidth)
            : strategy->compute(Cost::max().value);

    request.second.counting = 0u;

    request.second.blocking = 0u;
  }

  settings.minFSUsPerRequest = (*settings.requests.begin()).second.FSUs;

  for (const auto &request : settings.requests) {
    if (request.second.FSUs < settings.minFSUsPerRequest) {
      settings.minFSUsPerRequest = request.second.FSUs;
    }
  }

  settings.fragmentationStrategies = {
      {"absolute_fragmentation", std::make_shared<AbsoluteFragmentation>()},
      {"external_fragmentation", std::make_shared<ExternalFragmentation>()},
      {"entropy_based_fragmentation",
       std::make_shared<EntropyBasedFragmentation>(settings.minFSUsPerRequest)},
  };

  const auto probs = std::vector<double>(settings.requests.size(),
                                         1.0f / settings.requests.size());

  settings.probs = probs;

  const auto graph =
      Graph::from(json.Get<std::string>("params.topology").value());

  if (!graph.has_value()) {
    return std::nullopt;
  }

  settings.graph = graph.value();

  return settings;
}
