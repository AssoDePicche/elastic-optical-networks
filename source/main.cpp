#include <algorithm>
#include <chrono>
#include <ctime>
#include <format>
#include <fstream>
#include <iostream>
#include <optional>
#include <string>

#include "json.h"
#include "settings.h"
#include "simulation.h"
#include "statistics.h"

auto main(void) -> int {
  const std::string filename = "settings.json";

  const Json json(filename);

  Settings settings = Settings::From(json).value();

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
    std::string buffer{""};

    buffer.append("time, slots, accepted, fragmentation, entropy, blocking\n");

    std::for_each(snapshots.begin(), snapshots.end(),
                  [&buffer](const Snapshot &snapshot) {
                    buffer.append(std::format("{}\n", snapshot.str()));
                  });

    const auto time = std::time(nullptr);

    const auto localtime = std::localtime(&time);

    const std::string filename =
        std::format("{:02}-{:02}-{:04} {:02}h{:02}.csv", localtime->tm_mday,
                    localtime->tm_mon + 1, localtime->tm_year + 1900,
                    localtime->tm_hour, localtime->tm_min);

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

  for (const auto &snapshot : snapshots) {
    fragmentationStates.push_back(snapshot.fragmentation);

    entropyStates.push_back(snapshot.entropy);
  }

  const auto time = simulation.GetTime();

  const auto requestCount = simulation.GetRequestCount();

  std::string buffer{""};

  buffer.append("seed: {}\n", settings.seed);

  buffer.append(std::format("execution time (s): {}\n", execution_time));

  buffer.append(std::format("simulated time: {:.3f}\n", time));

  buffer.append(
      std::format("spectrum width (GHz): {:.2f}\n", settings.spectrumWidth));

  buffer.append(std::format("slot width (GHz): {:.2f}\n", settings.slotWidth));

  buffer.append(std::format("slots per link: {}\n", settings.bandwidth));

  buffer.append(std::format("mean fragmentation: {:.3f} ± {:.3f}\n",
                            MEAN(fragmentationStates),
                            STDDEV(fragmentationStates)));

  buffer.append(std::format("mean entropy: {:.3f} ± {:.3f}\n",
                            MEAN(entropyStates), STDDEV(entropyStates)));

  const double load = settings.arrivalRate / settings.serviceRate;

  buffer.append(std::format("load (E): {:.3f}\n", load));

  buffer.append(std::format("arrival rate: {:.3f}\n", settings.arrivalRate));

  buffer.append(std::format("service rate: {:.3f}\n", settings.serviceRate));

  buffer.append(std::format("grade of service: {:.3f}\n",
                            simulation.GetGradeOfService()));

  buffer.append(std::format("total requests: {}\n", requestCount));

  for (const auto &request : settings.requests) {
    const auto ratio = request.second.counting / requestCount;

    const auto gos = request.second.blocking / requestCount;

    buffer.append(std::format(
        "requests for {} FSU(s)\nratio: {:.3f}\ngrade of service: {:.3f}\n",
        request.second.resources, ratio, gos));
  }

  std::cout << buffer << std::endl;
}
