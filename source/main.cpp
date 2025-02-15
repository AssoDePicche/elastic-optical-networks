#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "settings.h"
#include "simulation.h"
#include "statistics.h"

auto main(const int argc, const char **argv) -> int {
  std::optional<Settings> typed_settings;

  if (!std::filesystem::exists("settings.json")) {
    std::vector<std::string> args;

    for (auto arg{1u}; arg < static_cast<std::size_t>(argc); ++arg) {
      args.push_back(std::string(argv[arg]));
    }

    typed_settings = Settings::from(args);
  } else {
    typed_settings = Settings::from("settings.json");
  }

  if (!typed_settings.has_value()) {
    return 1;
  }

  auto settings{typed_settings.value()};

  const auto start = std::chrono::system_clock::now();

  Simulation simulation(settings);

  while (simulation.has_next()) {
    simulation.next();
  }

  const auto end = std::chrono::system_clock::now();

  const auto runtime =
      std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

  std::string filename =
      "dataset" + std::to_string(simulation.blocking()) + ".csv";

  std::ofstream stream(filename);

  if (!stream.is_open()) {
    std::cerr << "Failed to open " << filename << '!' << std::endl;

    return 1;
  }

  stream << "time,bandwidth,accepted,";

  for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
    auto column =
        "from_" + std::to_string(source) + "_to_" + std::to_string(destination);

    stream << column << "_fragmentation," << column << "_entropy,";
  }

  stream << "blocking" << std::endl;

  std::vector<double> fragmentation_states;

  std::vector<double> entropy_states;

  for (const auto &snapshot : simulation.get_snapshots()) {
    stream << snapshot.str() << std::endl;

    fragmentation_states.insert(fragmentation_states.end(),
                                snapshot.fragmentation.begin(),
                                snapshot.fragmentation.end());

    entropy_states.insert(entropy_states.end(), snapshot.entropy.begin(),
                          snapshot.entropy.end());
  }

  stream.close();

  std::cout << "runtime: " << runtime << "s" << std::endl;

  std::cout << Report::from(simulation.group, settings).to_string()
            << std::endl;

  std::cout << "mean fragmentation: " << mean(fragmentation_states)
            << std::endl;

  std::cout << "stddev fragmentation: " << stddev(fragmentation_states)
            << std::endl;

  std::cout << "mean entropy: " << mean(entropy_states) << std::endl;

  std::cout << "stddev entropy: " << stddev(entropy_states) << std::endl;
}
