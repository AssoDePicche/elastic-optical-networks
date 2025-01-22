#include <iostream>
#include <string>

#include "settings.h"
#include "simulation.h"
#include "statistics.h"
#include "timer.h"

auto main(const int argc, const char **argv) -> int {
  std::vector<std::string> args;

  for (auto arg{1u}; arg < static_cast<std::size_t>(argc); ++arg) {
    args.push_back(std::string(argv[arg]));
  }

  const auto typed_settings{Settings::from(args)};

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

  std::vector<double> fragmentation_states;

  std::vector<double> entropy_states;

  for (const auto &snapshot : simulation.get_snapshots()) {
    fragmentation_states.push_back(snapshot.fragmentation);

    entropy_states.push_back(snapshot.entropy);
  }

  std::cout << "Execution time: " << timer.elapsed<std::chrono::seconds>()
            << std::endl;

  std::cout << "Simulation time: " << simulation.time << std::endl;

  std::cout << Report::from(simulation.group, settings).to_string()
            << std::endl;

  std::cout << "Mean fragmentation: " << MEAN(fragmentation_states)
            << std::endl;

  std::cout << "STDDEV fragmentation: " << STDDEV(fragmentation_states)
            << std::endl;

  std::cout << "Mean entropy: " << MEAN(entropy_states) << std::endl;

  std::cout << "STDDEV entropy: " << STDDEV(entropy_states) << std::endl;
}
