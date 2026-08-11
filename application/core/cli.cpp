#include "cli.h"

#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <stacktrace>
#include <string>

#include "configuration.h"
#include "document.h"
#include "file_system.h"
#include "kernel.h"

static double Benchmark(std::function<void()> callable) {
  const std::chrono::time_point<std::chrono::system_clock> start =
      std::chrono::system_clock::now();

  callable();

  const std::chrono::time_point<std::chrono::system_clock> end =
      std::chrono::system_clock::now();

  const std::chrono::seconds duration =
      std::chrono::duration_cast<std::chrono::seconds>(end - start);

  return duration.count();
}

namespace core {
struct CommandLineInterface::Implementation {
  int Run(const int argc, const char** argv) {
    try {
      const std::string configFile = GetConfigFilenameFromArgs(argc, argv);

      auto configuration = Configuration::From(configFile);

      if (!configuration) {
        return 1;
      }

      std::string dirname = "./temp";

      if (argc == 4) {
        configuration->arrivalRate = 1.0;

        configuration->serviceRate = std::stod(argv[2]);

        dirname = argv[3];
      }

      if (!file_system::create_directory(dirname)) {
        return 1;
      }

      Router router(configuration->graph);

      Kernel kernel(router, configuration);

      for (const auto iteration :
           std::ranges::views::iota(1u, configuration->iterations + 1u)) {
        std::clog << "Running iteration #" << iteration << std::endl;

        const auto execution_time = Benchmark([&]() { kernel.Run(); });

        std::clog << "Ended iteration #" << iteration << std::endl;

        const std::string report_filename =
            dirname + std::format("/{:02}_report.txt", iteration);

        Document document = kernel.GetReport();

        document.append("iteration: {}\n", iteration)
            .append("execution time (s): {}\n", execution_time);

        document.write(report_filename);

        if (configuration->exportDataset) {
          const std::string filename =
              dirname + std::format("/{:02}_dataset.csv", iteration);

          kernel.ExportDataset(filename);
        }

        kernel.Reset();
      }

      return 0;
    } catch (const std::exception& exception) {
      std::cerr << "Exception thrown: " << exception.what() << std::endl;

      return 1;
    }
  }

  std::string GetConfigFilenameFromArgs(const int argc, const char** argv) {
    if (argc == 2 || argc == 4) {
      return std::string(argv[1]);
    }

    return "resources/configuration/configuration.json";
  }
};

CommandLineInterface::CommandLineInterface()
    : pImpl(std::make_unique<Implementation>()) {}

CommandLineInterface::~CommandLineInterface() = default;

int CommandLineInterface::Run(const int argc, const char** argv) {
  return pImpl->Run(argc, argv);
}
}  // namespace core
