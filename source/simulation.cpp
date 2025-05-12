#include "simulation.h"

#include <algorithm>
#include <ctime>
#include <format>
#include <fstream>
#include <stdexcept>
#include <unordered_map>
#include <vector>

#include "distribution.h"
#include "event_queue.h"
#include "graph.h"
#include "logger.h"
#include "request.h"
#include "statistics.h"
#include "timer.h"

Snapshot::Snapshot(float t, int slots, bool acc, float frag, float entr,
                   float blocking)
    : time{t},
      slots{slots},
      accepted{acc},
      fragmentation{frag},
      entropy{entr},
      blocking{blocking} {}

auto Snapshot::str(void) const -> std::string {
  return std::to_string(time) + ", " + std::to_string(slots) + ", " +
         (accepted ? "True" : "False") + ", " + std::to_string(fragmentation) +
         ", " + std::to_string(entropy) + ", " + std::to_string(blocking);
}

Simulation::Simulation(Settings &settings)
    : settings{settings},
      queue{settings.arrivalRate, settings.serviceRate, settings.seed},
      distribution{settings.seed, settings.probs},
      kToIgnore{0.1 * settings.timeUnits},
      hashmap{make_hashmap(settings.graph, settings.bandwidth)},
      requestsKeys{} {
  requestsKeys.reserve(settings.requests.size());

  std::transform(settings.requests.begin(), settings.requests.end(),
                 std::back_inserter(requestsKeys),
                 [](const auto &pair) { return pair.first; });
  auto &firstRequest = settings.requests.at(requestsKeys[distribution.next()]);

  ++firstRequest.counting;

  queue.push(Request{random_path(settings.graph), firstRequest.resources}, time)
      .of_type(Signal::ARRIVAL);

  ++requestCount;

  timer.start();
}

bool Simulation::HasNext(void) const {
  return queue.top().value().time <= settings.timeUnits;
}

void Simulation::Next(void) {
  const std::unordered_map<std::string, unsigned> kModulationSlots = {
      {"BPSK", 1},   {"QPSK", 2},    {"8-QAM", 3},
      {"8-QAM", 3},  {"16-QAM", 4},  {"32-QAM", 5},
      {"64-QAM", 6}, {"128-QAM", 7}, {"256-QAM", 8},
  };

  if (settings.ignoreFirst && kToIgnore == queue.top().value().time &&
      !ignoredFirst) {
    ignoredFirst = true;

    requestCount = 0u;

    blockedCount = 0u;
  }

  auto event = queue.pop().value();

  time = event.time;

  INFO(std::format("Now: {}\n", time));

  if (event.signal == Signal::DEPARTURE) {
    --activeRequests;

    INFO(std::format("Request for {} slots departing", event.value.bandwidth));

    const auto keys{route_keys(event.value.route)};

    for (const auto &key : keys) {
      hashmap[key].deallocate(event.value.slice);

      INFO(hashmap[key].to_string());

      INFO(std::format("A (u): {} F (%): {}", hashmap[key].available(),
                       hashmap[key].fragmentation()));
    }

    return;
  }

  SpectrumAllocator allocator;

  for (auto &requestType : settings.requests) {
    const auto fsus = from_modulation(
        requestType.second.bandwidth,
        kModulationSlots.at(requestType.second.modulation), settings.slotWidth);

    if (fsus == event.value.bandwidth) {
      allocator =
          *requestType.second.allocator
               .target<std::optional<std::pair<unsigned int, unsigned int>> (*)(
                   const Spectrum &, unsigned int)>();

      break;
    }
  }

  auto accepted = false;

  if (activeRequests < settings.bandwidth &&
      dispatch_request(event.value, hashmap, allocator)) {
    ++activeRequests;

    queue.push(event.value, time).of_type(Signal::DEPARTURE);

    INFO(std::format("Accept request for {} FSU(s)", event.value.bandwidth));

    const auto keys{route_keys(event.value.route)};

    for (const auto &key : keys) {
      INFO(hashmap[key].to_string());

      INFO(std::format("A (u): {} F (%): {}", hashmap[key].available(),
                       hashmap[key].fragmentation()));
    }

    accepted = true;
  } else {
    INFO(std::format("Blocking request for {} FSU(s)", event.value.bandwidth));

    for (auto &request : settings.requests) {
      if (event.value.bandwidth != request.second.resources) {
        continue;
      }

      ++request.second.blocking;

      break;
    }

    ++blockedCount;
  }

  snapshots.push_back(Snapshot(time, event.value.bandwidth, accepted,
                               network_fragmentation(), entropy(),
                               blockedCount / requestCount));

  auto &request = settings.requests[requestsKeys[distribution.next()]];

  ++request.counting;

  queue.push(Request{random_path(settings.graph), request.resources}, time)
      .of_type(Signal::ARRIVAL);

  ++requestCount;
}

std::string Simulation::Report(void) {
  timer.stop();

  if (settings.exportDataset) {
    std::string buffer{""};

    buffer.append("time, slots, accepted, fragmentation, entropy, blocking\n");

    std::for_each(snapshots.begin(), snapshots.end(),
                  [&buffer](Snapshot &snapshot) {
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

  std::string buffer{""};

  buffer.append("seed: {}\n", settings.seed);

  const double execution_time = timer.elapsed<std::chrono::seconds>();

  buffer.append(std::format("execution time (s): {:.3f}\n", execution_time));

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
                            blockedCount / static_cast<double>(requestCount)));

  buffer.append(std::format("total requests: {}\n", requestCount));

  for (const auto &request : settings.requests) {
    const auto ratio =
        request.second.counting / static_cast<double>(requestCount);

    const auto gos =
        request.second.blocking / static_cast<double>(requestCount);

    buffer.append(std::format(
        "requests for {} FSU(s)\nratio: {:.3f}\ngrade of service: {:.3f}\n",
        request.second.resources, ratio, gos));
  }

  return buffer;
}

double Simulation::network_fragmentation(void) {
  auto fragmentation{0.0};

  for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    fragmentation += hashmap[key].fragmentation();
  }

  return (fragmentation / settings.graph.get_edges().size());
}

double Simulation::entropy(void) {
  auto free_slots = 0.0;

  auto occupied_slots = 0.0;

  for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    const auto spectrum = hashmap[key];

    free_slots += spectrum.available();

    occupied_slots += spectrum.size() - spectrum.available();
  }

  if (free_slots == 0.0 || occupied_slots == 0.0) {
    return 0.0;
  }

  free_slots /= settings.bandwidth;

  occupied_slots /= settings.bandwidth;

  return -(occupied_slots * std::log2(occupied_slots) +
           free_slots * std::log2(free_slots));
}
