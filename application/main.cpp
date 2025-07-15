#include <algorithm>
#include <exception>
#include <format>
#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

#include "date.h"
#include "distribution.h"
#include "json.h"
#include "math.h"
#include "settings.h"
#include "simulation.h"

auto main(void) -> int {
  try {
    const std::string filename = "resources/configuration/settings.json";

    const Json json(filename);

    Settings settings = Settings::From(json).value();

    const auto prng = PseudoRandomNumberGenerator::Instance();

    prng->set_seed(settings.seed);

    prng->set_exponential("arrival", settings.arrivalRate);

    prng->set_exponential("service", settings.serviceRate);

    prng->set_discrete("fsus", settings.probs);

    prng->set_uniform("routing", 0, settings.graph.size());

    Simulation simulation(settings);

    const auto start = std::chrono::system_clock::now();

    while (simulation.HasNext()) {
      simulation.Next();
    }

    const auto end = std::chrono::system_clock::now();

    const auto execution_time =
        std::chrono::duration_cast<std::chrono::seconds>(end - start).count();

    const auto snapshots = simulation.GetSnapshots();

    if (settings.exportDataset) {
      std::string buffer{"time,fsus,accepted,"};

      for (const auto &strategy : settings.fragmentationStrategies) {
        buffer.append(std::format("{},", strategy.first));
      }

      buffer.append("entropy,blocking\n");

      std::for_each(snapshots.begin(), snapshots.end(),
                    [&buffer](const Snapshot &snapshot) {
                      buffer.append(std::format("{}\n", snapshot.Serialize()));
                    });

      const auto datetime = DateTime::now();

      const std::string filename = std::format(
          "{:02}_{:02}_{:04}_{:02}h{:02}.csv", datetime.day, datetime.month,
          datetime.year, datetime.hour, datetime.minute);

      std::ofstream stream(filename);

      if (!stream.is_open()) {
        throw std::runtime_error(
            std::format("Failed to write {} file", filename));
      }

      stream << buffer;

      stream.close();
    }

    std::vector<double> fragmentationStates;

    std::vector<double> entropyStates;

    entropyStates.reserve(snapshots.size());

    for (const auto &snapshot : snapshots) {
      fragmentationStates.insert(fragmentationStates.end(),
                                 snapshot.fragmentation.begin(),
                                 snapshot.fragmentation.end());

      entropyStates.emplace_back(snapshot.entropy);
    }

    const auto time = simulation.GetTime();

    const auto requestCount = simulation.GetRequestCount();

    const std::unordered_map<std::string, std::vector<double>> units = {{
        {"fragmentation", fragmentationStates},
        {"entropy", entropyStates},
    }};

    std::string buffer{""};

    buffer.append(std::format("seed: {}\n", settings.seed));

    buffer.append(std::format("execution time (s): {}\n", execution_time));

    buffer.append(std::format("simulated time: {:.3f}\n", time));

    buffer.append(
        std::format("spectrum width (GHz): {:.2f}\n", settings.spectrumWidth));

    buffer.append(
        std::format("slot width (GHz): {:.2f}\n", settings.slotWidth));

    buffer.append(std::format("fsus per link: {}\n", settings.FSUsPerLink));

    for (const auto &[key, value] : units) {
      const auto unit = Unit::New(value.begin(), value.end());

      buffer.append(std::format("mean {}: {:.3f} ± {:.3f}\n", key, unit.mean,
                                unit.stddev));
    }

    const double load = settings.arrivalRate / settings.serviceRate;

    buffer.append(std::format("load (E): {:.3f}\n", load));

    buffer.append(std::format("arrival rate: {:.3f}\n", settings.arrivalRate));

    buffer.append(std::format("service rate: {:.3f}\n", settings.serviceRate));

    buffer.append(std::format("grade of service: {:.3f}\n",
                              simulation.GetGradeOfService()));

    buffer.append(std::format("total requests: {}\n", requestCount));

    for (const auto &request : settings.requests) {
      const auto ratio = request.second.counting / simulation.GetRequestCount();

      const auto gos = request.second.blocking / simulation.GetRequestCount();

      buffer.append(std::format(
          "requests for {} FSU(s)\nratio: {:.3f}\ngrade of service: "
          "{:.3f}\nnormalized load: {:.3f}\n",
          request.second.FSUs, ratio, gos,
          settings.arrivalRate * (static_cast<double>(request.second.FSUs) /
                                  settings.FSUsPerLink)));
    }

    const std::string report_filename = "report.txt";

    std::ofstream report_stream(report_filename);

    if (!report_stream.is_open()) {
      throw std::runtime_error(
          std::format("Failed to write {} file", report_filename));
    }

    report_stream << buffer;

    report_stream.close();

    std::cout << std::format("Simulation results wrote in {}\n",
                             report_filename);

  } catch (const std::exception &exception) {
    std::cerr << exception.what() << std::endl;
  }
}
