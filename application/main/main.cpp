#include <algorithm>
#include <chrono>
#include <ctime>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <optional>
#include <ranges>
#include <stacktrace>
#include <string>

#include <core/configuration.h>
#include <core/document.h>
#include <core/json.h>
#include <core/kernel.h>

[[nodiscard]] double benchmark(std::function<void()> callable) {
  const auto start = std::chrono::system_clock::now();

  callable();

  const auto end = std::chrono::system_clock::now();

  return std::chrono::duration_cast<std::chrono::microseconds>(end - start)
             .count() /
         1'000'000;
}

[[nodiscard]] std::string GetConfigFilenameFromArgs(const int argc,
                                                    const char **argv) {
  if (argc > 2) {
    return std::string(argv[2]);
  }

  return "resources/configuration/configuration.json";
}

[[nodiscard]] bool CreateDirectory(const std::string &path) {
  std::filesystem::path dir_path = path;

  std::error_code errorCode;

  if (std::filesystem::create_directories(dir_path, errorCode)) {
    std::clog << "Created " << dir_path << " dir" << std::endl;

    return true;
  }

  std::cerr << "Failed to create directory: " << errorCode.message()
            << std::endl;

  return false;
}

int main(const int argc, const char **argv) {
  if (argc != 4) {
    std::cerr << std::format(
        "You must inform:\n1. Service Rate\n2. Config file\n3. Output dir\n");

    return 1;
  }

  try {
    const std::string configFile = GetConfigFilenameFromArgs(argc, argv);

    const Json json(configFile);

    auto configuration = Configuration::From(json).value();

    if (argc > 1) {
      configuration->serviceRate = std::atof(argv[1]);
    }

    const std::string dirname = "./" + std::string(argv[3]);

    if (!CreateDirectory(dirname)) {
      return 1;
    }

    Kernel kernel(configuration);

    for (const auto iteration :
         std::ranges::views::iota(1u, configuration->iterations + 1u)) {
      std::clog << "Running iteration number " << iteration << std::endl;

      const auto execution_time = benchmark([&]() {
        while (kernel.HasNext()) {
          kernel.Next();
        }
      });

      std::clog << "Ended iteration number " << iteration << std::endl;

      const auto snapshots = kernel.GetSnapshots();

      const auto time = std::time(nullptr);

      const auto localtime = std::localtime(&time);

      const auto kernel_time = kernel.GetStatistics().time;

      const auto requestCount =
          static_cast<double>(kernel.GetStatistics().total_requests);

      Document document;

      document
          .append("created at: {:02}/{:02}/{:04} {:02}h{:02}\n",
                  localtime->tm_mday, localtime->tm_mon + 1,
                  localtime->tm_year + 1900, localtime->tm_hour,
                  localtime->tm_min)
          .append("iteration: {}\n", iteration)
          .append("seed: {}\n", kernel.GetPseudoRandomNumberGenerator()->seed())
          .append("execution time (s): {}\n", execution_time)
          .append("simulated time: {:.3f}\n", kernel_time)
          .append("spectrum width (GHz): {:.2f}\n",
                  kernel.GetConfiguration()->spectrumWidth)
          .append("slot width (GHz): {:.2f}\n",
                  kernel.GetConfiguration()->slotWidth)
          .append("fsus per link: {}\n",
                  kernel.GetConfiguration()->FSUsPerLink);

      const double load = kernel.GetConfiguration()->arrivalRate /
                          kernel.GetConfiguration()->serviceRate;

      document.append("load (E): {:.3f}\n", load)
          .append("arrival rate: {:.3f}\n",
                  kernel.GetConfiguration()->arrivalRate)
          .append("service rate: {:.3f}\n",
                  kernel.GetConfiguration()->serviceRate)
          .append("grade of service: {:.3f}\n",
                  kernel.GetStatistics().GradeOfService())
          .append("total requests: {}\n", requestCount);

      for (const auto &[_, requestType] :
           kernel.GetConfiguration()->requestTypes) {
        const auto ratio = requestType.counting / requestCount;

        const auto gos = requestType.blocking / requestCount;

        const auto normalized_load = kernel.GetConfiguration()->arrivalRate *
                                     (static_cast<double>(requestType.FSUs) /
                                      kernel.GetConfiguration()->FSUsPerLink);

        document.append("requests for {} FSU(s)\n", requestType.FSUs)
            .append("ratio: {:.3f}\n", ratio)
            .append("grade of service: {:.3f}\n", gos)
            .append("normalized load: {:.3f}\n", normalized_load);
      }

      const std::string report_filename =
          dirname + std::format("/{:02}_report.txt", iteration);

      document.write(report_filename);

      kernel.Reset();

      if (!kernel.GetConfiguration()->exportDataset) {
        continue;
      }

      std::string buffer{
          "time,absolute_fragmentation,entropy,external_fragmentation,grade_of_"
          "service,slot_blocking_probability,active_requests\n"};

      std::for_each(snapshots.begin(), snapshots.end(),
                    [&buffer](const Statistics &snapshot) {
                      buffer.append(std::format("{}\n", snapshot.Serialize()));
                    });

      const std::string filename =
          dirname + std::format("/{:02}_dataset.csv", iteration);

      std::ofstream stream(filename);

      if (!stream.is_open()) {
        throw std::runtime_error(
            std::format("Failed to write {} file", filename));
      }

      stream << buffer;

      stream.close();
    }
  } catch (const std::exception &exception) {
    std::cerr << "Exception thrown: " << exception.what() << std::endl;

    return 1;
  }

  return 0;
}
