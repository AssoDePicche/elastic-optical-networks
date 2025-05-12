#include <iostream>
#include <optional>
#include <string>

#include "json.h"
#include "settings.h"
#include "simulation.h"

auto main(void) -> int {
  const std::string filename = "settings.json";

  const Json json(filename);

  std::optional<Settings> settings = Settings::From(json);

  Simulation simulation(settings.value());

  while (simulation.HasNext()) {
    simulation.Next();
  }

  std::cout << simulation.Report() << std::endl;
}
