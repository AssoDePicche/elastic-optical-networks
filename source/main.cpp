#include <filesystem>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "settings.h"
#include "simulation.h"
#include "statistics.h"
#include "timer.h"

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

  Timer timer;

  timer.start();

  Simulation simulation(settings);

  while (simulation.has_next()) {
    simulation.next();
  }

  timer.stop();

  std::string filename =
      "dataset" + std::to_string(simulation.blocking()) + ".csv";

  std::ofstream stream(filename);

  if (!stream.is_open()) {
    std::cerr << "Failed to open " << filename << '!' << std::endl;

    return 1;
  }

  stream << "time,bandwidth,accepted,fragmentation,entropy,blocking"
         << std::endl;

  std::vector<double> fragmentation_states;

  std::vector<double> entropy_states;

  for (const auto &snapshot : simulation.get_snapshots()) {
    stream << snapshot.str() << std::endl;

    fragmentation_states.push_back(snapshot.fragmentation);

    entropy_states.push_back(snapshot.entropy);
  }

  stream.close();

  std::cout << "Runtime: " << timer.elapsed<std::chrono::seconds>() << "s"
            << std::endl;

  std::cout << "Simulated time units: " << simulation.time << std::endl;

  std::cout << Report::from(simulation.group, settings).to_string()
            << std::endl;

  std::cout << "AVG fragmentation: " << MEAN(fragmentation_states) << std::endl;

  std::cout << "STDDEV fragmentation: " << STDDEV(fragmentation_states)
            << std::endl;

  std::cout << "AVG entropy: " << MEAN(entropy_states) << std::endl;

  std::cout << "STDDEV entropy: " << STDDEV(entropy_states) << std::endl;
}
