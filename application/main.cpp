#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <stacktrace>
#include <string>

#include "configuration.h"
#include "document.h"
#include "json.h"
#include "kernel.h"

std::string GetConfigFilenameFromArgs(const int argc, const char **argv) {
  if (argc > 1) {
    return std::string(argv[1]);
  }

  return "resources/configuration/configuration.json";
}

int main(const int argc, const char **argv) {
  try {
    const std::string configFile = GetConfigFilenameFromArgs(argc, argv);

    const Json json(configFile);

    auto configuration = Configuration::From(json).value();

    std::cout << "Initializing simulation with " << configFile << std::endl;

    Kernel kernel(configuration);

    for (const auto iteration :
         std::ranges::views::iota(1u, configuration->iterations + 1u)) {
      const auto start = std::chrono::system_clock::now();

      while (kernel.HasNext()) {
        kernel.Next();
      }

      const auto end = std::chrono::system_clock::now();

      const auto execution_time =
          std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

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
                  configuration->spectrumWidth)
          .append("slot width (GHz): {:.2f}\n", configuration->slotWidth)
          .append("fsus per link: {}\n", configuration->FSUsPerLink);

      const double load =
          configuration->arrivalRate / configuration->serviceRate;

      document.append("load (E): {:.3f}\n", load)
          .append("arrival rate: {:.3f}\n", configuration->arrivalRate)
          .append("service rate: {:.3f}\n", configuration->serviceRate)
          .append("grade of service: {:.3f}\n",
                  kernel.GetStatistics().GradeOfService())
          .append("total requests: {}\n", requestCount);

      for (const auto &[_, requestType] : configuration->requestTypes) {
        const auto ratio = requestType.counting / requestCount;

        const auto gos = requestType.blocking / requestCount;

        const auto normalized_load = configuration->arrivalRate *
                                     (static_cast<double>(requestType.FSUs) /
                                      configuration->FSUsPerLink);

        document.append("requests for {} FSU(s)\n", requestType.FSUs)
            .append("ratio: {:.3f}\n", ratio)
            .append("grade of service: {:.3f}\n", gos)
            .append("normalized load: {:.3f}\n", normalized_load);
      }

      const auto report_filename =
          std::format("resources/temp/{:02}_report.txt", iteration);

      document.write(report_filename);

      std::cout << std::format("Simulation results wrote in {}\n",
                               report_filename);

      kernel.Reset();

      if (!configuration->exportDataset) {
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
          std::format("resources/temp/{:02}_dataset.csv", iteration);

      std::ofstream stream(filename);

      if (!stream.is_open()) {
        throw std::runtime_error(
            std::format("Failed to write {} file", filename));
      }

      stream << buffer;

      stream.close();

      std::cout << std::format("Simulation data wrote in {}\n", filename);
    }
  } catch (const std::exception &exception) {
    std::cerr << "Exception thrown: " << exception.what() << std::endl;

    return 1;
  }

  return 0;
}
