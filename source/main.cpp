#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "settings.h"
#include "simulation.h"

auto main(const int argc, const char **argv) -> int {
  const std::string filename = "settings.json";

  std::optional<Settings> settings;

  if (!std::filesystem::exists(filename)) {
    std::vector<std::string> args;

    for (auto arg{1u}; arg < static_cast<std::size_t>(argc); ++arg) {
      args.push_back(std::string(argv[arg]));
    }

    settings = Settings::from(args);
  } else {
    settings = Settings::from(filename);
  }

  if (!settings.has_value()) {
    return 1;
  }

  std::cout << simulation(settings.value()) << std::endl;
}
