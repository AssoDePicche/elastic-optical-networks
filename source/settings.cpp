#include "settings.h"

#include <iostream>
#include <map>
#include <set>

#include "json.h"
#include "parser.h"

auto Settings::from(const std::string &filename) -> std::optional<Settings> {
  const auto container = read_json(filename);

  if (!container.has_value()) {
    return std::nullopt;
  }

  const auto json = container.value();

  Settings settings;

  settings.bandwidth = json["simulation"]["bandwidth"];

  settings.time_units = json["simulation"]["time-units"];

  settings.arrival_rate = json["simulation"]["arrival-rate"];

  settings.service_rate = json["simulation"]["service-rate"];

  const auto graph_file = json["simulation"]["graph"];

  const auto graph_container = Graph::from(graph_file);

  if (!graph_container.has_value()) {
    return std::nullopt;
  }

  settings.graph = graph_container.value();

  settings.seed = json["simulation"]["seed"];

  return settings;
}

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

  settings.time_units = str_to_double(parser.parse("--time-units"));

  settings.arrival_rate = str_to_double(parser.parse("--arrival-rate"));

  if (settings.arrival_rate <= 0.0) {
    return std::nullopt;
  }

  settings.service_rate = str_to_double(parser.parse("--service-rate"));

  if (settings.service_rate <= 0.0) {
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

  settings.spectrum_allocator =
      spectrum_allocation_strategies.at(spectrum_allocation_strategy);

  settings.seed = std::stoul(parser.parse("--seed"));

  return settings;
}
