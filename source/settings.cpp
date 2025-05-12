#include "settings.h"

#include "request.h"

auto Settings::From(const Json &json) -> std::optional<Settings> {
  const std::unordered_map<std::string, SpectrumAllocator>
      spectrum_allocation_strategies{{"best-fit", best_fit},
                                     {"first-fit", first_fit},
                                     {"last-fit", last_fit},
                                     {"random-fit", random_fit},
                                     {"worst-fit", worst_fit}};

  Settings settings;

  settings.enableLogging = json.Get<bool>("enable-logging").value();

  settings.exportDataset = json.Get<bool>("export-dataset").value();

  settings.ignoreFirst = json.Get<bool>("params.ignore-first").value();

  settings.timeUnits = json.Get<double>("params.simulation-duration").value();

  settings.arrivalRate = json.Get<double>("params.arrival-rate").value();

  settings.serviceRate = json.Get<double>("params.service-rate").value();

  settings.seed = json.Get<unsigned>("params.seed").value();

  settings.spectrumWidth = json.Get<double>("params.spectrum-width").value();

  settings.slotWidth = json.Get<double>("params.slot-width").value();

  settings.bandwidth = settings.spectrumWidth / settings.slotWidth;

  const auto requests =
      json.Get<std::vector<nlohmann::json>>("params.requests");

  for (const auto &row : requests.value()) {
    RequestType request;

    request.type = row["type"];

    request.modulation = row["modulation"];

    request.bandwidth = row["bandwidth"];

    request.allocator = spectrum_allocation_strategies.at(row["allocator"]);

    settings.requests[request.type] = request;
  }

  const std::unordered_map<std::string, unsigned> kModulationSlots = {
      {"BPSK", 1},   {"QPSK", 2},    {"8-QAM", 3},
      {"8-QAM", 3},  {"16-QAM", 4},  {"32-QAM", 5},
      {"64-QAM", 6}, {"128-QAM", 7}, {"256-QAM", 8},
  };

  for (auto &request : settings.requests) {
    request.second.resources = from_modulation(
        request.second.bandwidth,
        kModulationSlots.at(request.second.modulation), settings.slotWidth);

    request.second.counting = 0u;

    request.second.blocking = 0u;
  }

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
