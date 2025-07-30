#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stacktrace>
#include <string>

#include "configuration.h"
#include "document.h"
#include "json.h"
#include "kernel.h"

int main(void) {
  try {
    const Json json("resources/configuration/configuration.json");

    auto configuration = Configuration::From(json).value();

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

      const auto DAY = localtime->tm_mday;

      const auto MONTH = localtime->tm_mon + 1;

      const auto YEAR = localtime->tm_year + 1900;

      const auto HOUR = localtime->tm_hour;

      const auto MINUTE = localtime->tm_min;

      const auto kernel_time = kernel.GetTime();

      const auto requestCount = kernel.GetRequestCount();

      Document document;

      document.append("iteration: {}\n", iteration)
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
          .append("grade of service: {:.3f}\n", kernel.GetGradeOfService())
          .append("total requests: {}\n", requestCount);

      for (const auto &request : configuration->requests) {
        const auto ratio = request.second.counting / kernel.GetRequestCount();

        const auto gos = request.second.blocking / kernel.GetRequestCount();

        const auto normalized_load = configuration->arrivalRate *
                                     (static_cast<double>(request.second.FSUs) /
                                      configuration->FSUsPerLink);

        document.append("requests for {} FSU(s)\n", request.second.FSUs)
            .append("ratio: {:.3f}\n", ratio)
            .append("grade of service: {:.3f}\n", gos)
            .append("normalized load: {:.3f}\n", normalized_load);
      }

      const auto report_filename =
          std::format("resources/temp/{}_{:02}_{:02}_{:04}_{:02}h{:02}.txt",
                      iteration, DAY, MONTH, YEAR, HOUR, MINUTE);

      document.write(report_filename);

      std::cout << std::format("Simulation results wrote in {}\n",
                               report_filename);

      kernel.Reset();

      if (!configuration->exportDataset) {
        continue;
      }

      std::string buffer{"time,fsus,accepted,"};

      for (const auto &strategy : configuration->fragmentationStrategies) {
        buffer.append(std::format("{},", strategy.first));
      }

      buffer.append("blocking\n");

      std::for_each(snapshots.begin(), snapshots.end(),
                    [&buffer](const Snapshot &snapshot) {
                      buffer.append(std::format("{}\n", snapshot.Serialize()));
                    });

      const std::string filename =
          std::format("resources/temp/{}_{:02}_{:02}_{:04}_{:02}h{:02}.csv",
                      iteration, DAY, MONTH, YEAR, HOUR, MINUTE);

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
