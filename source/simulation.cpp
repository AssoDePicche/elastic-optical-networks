#include "simulation.h"

#include <format>
#include <tuple>
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

auto simulation(Settings &settings) -> std::string {
  const auto kToIgnore = 0.1 * settings.timeUnits;

  bool ignoredFirst = false;

  auto requestCount = 0u;

  auto blockedCount = 0u;

  auto activeRequests = 0u;

  auto hashmap{make_hashmap(settings.graph, settings.bandwidth)};

  EventQueue<Request> queue{settings.arrivalRate, settings.serviceRate,
                            settings.seed};

  std::vector<double> probs(settings.requests.size(),
                            1.0f / settings.requests.size());

  const std::unordered_map<std::string, unsigned> kModulationSlots = {
      {"BPSK", 1},   {"QPSK", 2},    {"8-QAM", 3},
      {"8-QAM", 3},  {"16-QAM", 4},  {"32-QAM", 5},
      {"64-QAM", 6}, {"128-QAM", 7}, {"256-QAM", 8},
  };

  std::vector<unsigned> slots;

  slots.reserve(settings.requests.size());

  for (const auto &request : settings.requests) {
    slots.emplace_back(from_modulation(
        request.second.bandwidth,
        kModulationSlots.at(request.second.modulation), settings.slotWidth));
  }

  std::vector<std::tuple<unsigned, unsigned, unsigned>> container;

  for (auto index = 0u; index < probs.size(); ++index) {
    container.push_back({slots[index], 0, 0});
  }

  Discrete distribution{settings.seed, probs};

  std::vector<Snapshot> snapshots{};

  const auto entropy = [&](void) {
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
  };

  const auto network_fragmentation = [&](void) {
    auto fragmentation{0.0};

    for (const auto &[source, destination, cost] : settings.graph.get_edges()) {
      const auto key = make_key(source, destination);

      fragmentation += hashmap[key].fragmentation();
    }

    return (fragmentation / settings.graph.get_edges().size());
  };

  Timer timer;

  timer.start();

  auto time{0.0};

  auto &[resource, counting, blocking] = container[distribution.next()];

  ++counting;

  queue.push(Request{random_path(settings.graph), resource}, time)
      .of_type(Signal::ARRIVAL);

  ++requestCount;

  while (queue.top().value().time <= settings.timeUnits) {
    if (settings.ignoreFirst && kToIgnore == queue.top().value().time &&
        !ignoredFirst) {
      ignoredFirst = true;

      requestCount = 0u;

      blockedCount = 0u;
    }

    const auto top{queue.pop()};

    auto [now, signal, request] = top.value();

    time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::DEPARTURE) {
      --activeRequests;

      INFO("Request for " + std::to_string(request.bandwidth) +
           " slots departing");

      const auto keys{route_keys(request.route)};

      for (const auto &key : keys) {
        hashmap[key].deallocate(request.slice);

        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

      continue;
    }

    auto allocator = first_fit;

    for (auto &requestType : settings.requests) {
      const auto fsus =
          from_modulation(requestType.second.bandwidth,
                          kModulationSlots.at(requestType.second.modulation),
                          settings.slotWidth);

      if (fsus == request.bandwidth) {
        allocator = *requestType.second.allocator.target<
            std::optional<std::pair<unsigned int, unsigned int>> (*)(
                const Spectrum &, unsigned int)>();

        break;
      }
    }

    auto accepted = false;

    if (activeRequests < settings.bandwidth &&
        dispatch_request(request, hashmap, allocator)) {
      ++activeRequests;

      queue.push(request, now).of_type(Signal::DEPARTURE);

      INFO("Accept request for " + std::to_string(request.bandwidth) +
           " slots");

      const auto keys{route_keys(request.route)};

      for (const auto &key : keys) {
        INFO(hashmap[key].to_string());

        INFO("A (u): " + std::to_string(hashmap[key].available()) +
             " F (%): " + std::to_string(hashmap[key].fragmentation()));
      }

      accepted = true;
    } else {
      INFO("Blocking request for " + std::to_string(request.bandwidth) +
           " slots");

      for (auto &[res, c, b] : container) {
        if (res != request.bandwidth) {
          continue;
        }

        ++b;
      }

      ++blockedCount;
    }

    snapshots.push_back(Snapshot(now, request.bandwidth, accepted,
                                 network_fragmentation(), entropy(),
                                 blockedCount / requestCount));

    auto &[_Resource, _Counting, _Blocking] = container[distribution.next()];

    ++_Counting;

    queue.push(Request{random_path(settings.graph), _Resource}, now)
        .of_type(Signal::ARRIVAL);

    ++requestCount;
  }

  timer.stop();

  std::string str{""};

  str.append("time,slots,accepted,fragmentation,entropy,blocking\n");

  std::vector<double> fragmentationStates;

  std::vector<double> entropyStates;

  for (const auto &snapshot : snapshots) {
    str.append(snapshot.str() + "\n");

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

  for (const auto &[_r, _c, _b] : container) {
    const auto ratio = _c / static_cast<double>(requestCount);

    const auto gos = _b / static_cast<double>(requestCount);

    buffer.append(std::format(
        "requests for {} FSU(s)\nratio: {:.3f}\ngrade of service: {:.3f}\n", _r,
        ratio, gos));
  }

  return buffer;
}
