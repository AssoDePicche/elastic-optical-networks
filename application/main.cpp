#include <algorithm>
#include <format>
#include <fstream>
#include <iostream>
#include <ranges>
#include <stacktrace>
#include <string>

#include "configuration.h"
#include "date.h"
#include "document.h"
#include "json.h"
#include "kernel.h"

int main(void) {
  try {
    const Json json("resources/configuration/configuration.json");

    Configuration configuration = Configuration::From(json).value();

    Kernel kernel(configuration);

    for (const auto iteration :
         std::ranges::views::iota(1u, configuration.iterations + 1u)) {
      const auto start = std::chrono::system_clock::now();

      while (kernel.HasNext()) {
        kernel.Next();
      }

      const auto end = std::chrono::system_clock::now();

      const auto execution_time =
          std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

      const auto snapshots = kernel.GetSnapshots();

      if (configuration.exportDataset) {
        std::string buffer{"time,fsus,accepted,"};

        for (const auto &strategy : configuration.fragmentationStrategies) {
          buffer.append(std::format("{},", strategy.first));
        }

        buffer.append("blocking\n");

        std::for_each(
            snapshots.begin(), snapshots.end(),
            [&buffer](const Snapshot &snapshot) {
              buffer.append(std::format("{}\n", snapshot.Serialize()));
            });

        const auto datetime = DateTime::now();

        const std::string filename = std::format(
            "resources/temp/{}_iteration_{:02}_{:02}_{:04}_{:02}h{:02}.csv",
            iteration, datetime.day, datetime.month, datetime.year,
            datetime.hour, datetime.minute);

        std::ofstream stream(filename);

        if (!stream.is_open()) {
          throw std::runtime_error(
              std::format("Failed to write {} file", filename));
        }

        stream << buffer;

        stream.close();
      }

      const auto time = kernel.GetTime();

      const auto requestCount = kernel.GetRequestCount();

      Document document;

      document.append("iteration: {}\n", iteration)
          .append("seed: {}\n", kernel.GetPseudoRandomNumberGenerator()->seed())
          .append("execution time (s): {}\n", execution_time)
          .append("simulated time: {:.3f}\n", time)
          .append("spectrum width (GHz): {:.2f}\n", configuration.spectrumWidth)
          .append("slot width (GHz): {:.2f}\n", configuration.slotWidth)
          .append("fsus per link: {}\n", configuration.FSUsPerLink);

      const double load = configuration.arrivalRate / configuration.serviceRate;

      document.append("load (E): {:.3f}\n", load)
          .append("arrival rate: {:.3f}\n", configuration.arrivalRate)
          .append("service rate: {:.3f}\n", configuration.serviceRate)
          .append("grade of service: {:.3f}\n", kernel.GetGradeOfService())
          .append("total requests: {}\n", requestCount);

      for (const auto &request : configuration.requests) {
        const auto ratio = request.second.counting / kernel.GetRequestCount();

        const auto gos = request.second.blocking / kernel.GetRequestCount();

        const auto normalized_load = configuration.arrivalRate *
                                     (static_cast<double>(request.second.FSUs) /
                                      configuration.FSUsPerLink);

        document.append("requests for {} FSU(s)\n", request.second.FSUs)
            .append("ratio: {:.3f}\n", ratio)
            .append("grade of service: {:.3f}\n", gos)
            .append("normalized load: {:.3f}\n", normalized_load);
      }

      const std::string report_filename =
          std::format("resources/temp/{}_iteration_report.txt", iteration);

      document.write(report_filename);

      std::cout << std::format("Simulation results wrote in {}\n",
                               report_filename);

      kernel.Reset();
    }
  } catch (const std::exception &exception) {
    std::cerr << "Exception thrown: " << exception.what() << std::endl;

    return 1;
  }

  return 0;
}
