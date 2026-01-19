#include <core/configuration.h>
#include <core/document.h>
#include <core/file_system.h>
#include <core/json.h>
#include <core/kernel.h>

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

[[nodiscard]] double benchmark(std::function<void()> callable) {
  const auto start = std::chrono::system_clock::now();

  callable();

  const auto end = std::chrono::system_clock::now();

  return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
             .count() /
         1'000'000;
}

[[nodiscard]] std::string GetConfigFilenameFromArgs(const int argc,
                                                    const char** argv) {
  if (argc > 2) {
    return std::string(argv[2]);
  }

  return "resources/configuration/configuration.json";
}

int main(const int argc, const char** argv) {
  try {
    const std::string configFile = GetConfigFilenameFromArgs(argc, argv);

    const core::Json json(configFile);

    auto configuration = core::Configuration::From(json).value();

    if (argc > 1) {
      configuration->serviceRate = std::atof(argv[1]);
    }

    const std::string dirname = "./" + std::string(argv[3]);

    if (!core::FileSystem::CreateDirectory(dirname)) {
      return 1;
    }

    core::Kernel kernel(configuration);

    for (const auto iteration :
         std::ranges::views::iota(1u, configuration->iterations + 1u)) {
      std::clog << "Running iteration number " << iteration << std::endl;

      const auto execution_time = benchmark([&]() { kernel.Run(); });

      std::clog << "Ended iteration number " << iteration << std::endl;

      const std::string report_filename =
          dirname + std::format("/{:02}_report.txt", iteration);

      core::Document document = kernel.GetReport();

      document.append("iteration: {}\n", iteration)
          .append("execution time (s): {}\n", execution_time);

      document.write(report_filename);

      if (!kernel.GetConfiguration()->exportDataset) {
        const std::string filename =
            dirname + std::format("/{:02}_dataset.csv", iteration);

        kernel.ExportDataset(filename);
      }

      kernel.Reset();
    }
  } catch (const std::exception& exception) {
    std::cerr << "Exception thrown: " << exception.what() << std::endl;

    return 1;
  }

  return 0;
}
