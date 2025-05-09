#include "settings.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>

#include "json.h"
#include "parser.h"

auto Settings::from(const std::vector<std::string> &argv)
    -> std::optional<Settings> {
  const std::set<std::string> args{
      "--bandwidth",          "--service-rate", "--arrival-rate", "--topology",
      "--spectrum-allocator", "--seed",         "--time-units"};

  if (argv.size() < args.size()) {
    std::cerr << "You must pass all the arguments:" << std::endl;

    for (const auto &arg : args) {
      std::cerr << arg << std::endl;
    }

    return std::nullopt;
  }

  const Parser parser{argv};

  for (const auto &arg : args) {
    if (!parser.contains(arg)) {
      std::cerr << "You must pass the " << arg << " argument" << std::endl;

      return std::nullopt;
    }
  }

  const auto str_to_unsigned = [](const std::string &str) {
    return static_cast<unsigned>(std::atoi(str.c_str()));
  };

  const auto str_to_double = [](const std::string &str) {
    return std::stod(str);
  };

  Settings settings;

  settings.bandwidth = str_to_unsigned(parser.parse("--bandwidth"));

  settings.timeUnits = str_to_double(parser.parse("--time-units"));

  settings.arrivalRate = str_to_double(parser.parse("--arrival-rate"));

  if (settings.arrivalRate <= 0.0) {
    return std::nullopt;
  }

  settings.serviceRate = str_to_double(parser.parse("--service-rate"));

  if (settings.serviceRate <= 0.0) {
    return std::nullopt;
  }

  const auto filename{parser.parse("--topology")};

  const auto container{Graph::from(filename)};

  if (!container.has_value()) {
    std::cerr << "Unable to read the " << filename << " file." << std::endl;

    return std::nullopt;
  }

  settings.graph = container.value();

  const auto spectrum_allocation_strategy{parser.parse("--spectrum-allocator")};

  const std::map<std::string, SpectrumAllocator> spectrum_allocation_strategies{
      {"best-fit", best_fit},
      {"first-fit", first_fit},
      {"last-fit", last_fit},
      {"random-fit", random_fit},
      {"worst-fit", worst_fit}};

  if (spectrum_allocation_strategies.find(spectrum_allocation_strategy) ==
      spectrum_allocation_strategies.end()) {
    return std::nullopt;
  }

  settings.spectrumAllocator =
      spectrum_allocation_strategies.at(spectrum_allocation_strategy);

  settings.seed = std::stoul(parser.parse("--seed"));

  return settings;
}

auto Settings::from(const std::string &filename) -> std::optional<Settings> {
  const auto json = read_json(filename);

  if (!json.has_value()) {
    return std::nullopt;
  }

  const std::map<std::string, SpectrumAllocator> spectrum_allocation_strategies{
      {"best-fit", best_fit},
      {"first-fit", first_fit},
      {"last-fit", last_fit},
      {"random-fit", random_fit},
      {"worst-fit", worst_fit}};

  const auto buffer = json.value();

  Settings settings;

  settings.enableLogging = buffer["enable-logging"];

  settings.exportDataset = buffer["export-dataset"];

  settings.ignoreFirst = buffer["params"]["ignore-first"];

  settings.timeUnits = buffer["params"]["simulation-duration"];

  settings.arrivalRate = buffer["params"]["arrival-rate"];

  settings.serviceRate = buffer["params"]["service-rate"];

  settings.seed = buffer["params"]["seed"];

  settings.spectrumWidth = buffer["params"]["spectrum-width"];

  settings.slotWidth = buffer["params"]["slot-width"];

  settings.bandwidth = settings.spectrumWidth / settings.slotWidth;

  for (const auto &row : buffer["params"]["requests"]) {
    RequestType request;

    request.type = row["type"];

    request.modulation = row["modulation"];

    request.bandwidth = row["bandwidth"];

    request.allocator = spectrum_allocation_strategies.at(row["allocator"]);

    settings.requests[request.type] = request;
  }

  const auto graph = Graph::from(buffer["params"]["topology"]);

  if (!graph.has_value()) {
    return std::nullopt;
  }

  settings.graph = graph.value();

  return settings;
}
