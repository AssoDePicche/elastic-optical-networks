#include "simulation.h"

#include <format>
#include <unordered_map>

#include "event_queue.h"
#include "graph.h"
#include "group.h"
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
         (accepted ? "true" : "false") + ", " + std::to_string(fragmentation) +
         ", " + std::to_string(entropy) + ", " + std::to_string(blocking);
}

auto simulation(Settings &settings) -> std::string {
  bool ignored_first = false;

  auto request_count = 0u;

  auto blocked_count = 0u;

  auto active_requests{0u};

  auto hashmap{make_hashmap(settings.graph, settings.bandwidth)};

  EventQueue<Request> queue{settings.arrival_rate, settings.service_rate,
                            settings.seed};

  std::vector<double> probs(settings.requests.size(),
                            1.0f / settings.requests.size());

  std::unordered_map<std::string, unsigned> modulation_slots = {
      {"BPSK", 1},   {"QPSK", 2},    {"8-QAM", 3},
      {"8-QAM", 3},  {"16-QAM", 4},  {"32-QAM", 5},
      {"64-QAM", 6}, {"128-QAM", 7}, {"256-QAM", 8},
  };

  std::vector<unsigned> slots;

  slots.reserve(settings.requests.size());

  for (const auto &request : settings.requests) {
    slots.emplace_back(from_modulation(
        request.second.bandwidth, modulation_slots[request.second.modulation],
        settings.slot_width));
  }

  Group group{settings.seed, probs, slots};

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

  queue.push(Request{random_path(settings.graph), group.next()}, time)
      .of_type(Signal::ARRIVAL);

  ++request_count;

  while (queue.top().value().time <= settings.time_units) {
    if (static_cast<unsigned>(0.1 * settings.time_units) == request_count &&
        !ignored_first) {
      ignored_first = true;

      request_count = 0u;

      blocked_count = 0u;
    }

    const auto top{queue.pop()};

    auto [now, signal, request] = top.value();

    time = now;

    INFO("Now: " + std::to_string(now));

    if (signal == Signal::DEPARTURE) {
      --active_requests;

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
      const auto fsus = from_modulation(
          requestType.second.bandwidth,
          modulation_slots[requestType.second.modulation], settings.slot_width);

      if (fsus == request.bandwidth) {
        allocator = *requestType.second.allocator.target<
            std::optional<std::pair<unsigned int, unsigned int>> (*)(
                const Spectrum &, unsigned int)>();

        break;
      }
    }

    auto accepted = false;

    if (active_requests < settings.bandwidth &&
        dispatch_request(request, hashmap, allocator)) {
      ++active_requests;

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

      group.count_blocking(request.bandwidth);

      ++blocked_count;
    }

    snapshots.push_back(Snapshot(now, request.bandwidth, accepted,
                                 network_fragmentation(), entropy(),
                                 group.blocking()));

    queue.push(Request{random_path(settings.graph), group.next()}, now)
        .of_type(Signal::ARRIVAL);

    ++request_count;
  }

  timer.stop();

  std::string str{""};

  str.append("time,slots,accepted,fragmentation,entropy,blocking\n");

  std::vector<double> fragmentation_states;

  std::vector<double> entropy_states;

  for (const auto &snapshot : snapshots) {
    str.append(snapshot.str() + "\n");

    fragmentation_states.push_back(snapshot.fragmentation);

    entropy_states.push_back(snapshot.entropy);
  }

  std::string buffer{""};

  buffer.append("seed: {}\n", settings.seed);

  const double execution_time = timer.elapsed<std::chrono::seconds>();

  buffer.append(std::format("execution time (s): {:.3f}\n", execution_time));

  buffer.append(std::format("simulated time: {:.3f}\n", time));

  buffer.append(
      std::format("spectrum width (GHz): {:.2f}\n", settings.spectrum_width));

  buffer.append(std::format("slot width (GHz): {:.2f}\n", settings.slot_width));

  buffer.append(std::format("slots per link: {}\n", settings.bandwidth));

  buffer.append(std::format("mean fragmentation: {:.3f} ± {:.3f}\n",
                            MEAN(fragmentation_states),
                            STDDEV(fragmentation_states)));

  buffer.append(std::format("mean entropy: {:.3f} ± {:.3f}\n",
                            MEAN(entropy_states), STDDEV(entropy_states)));

  const double load = settings.arrival_rate / settings.service_rate;

  buffer.append(std::format("load (E): {:.3f}\n", load));

  buffer.append(std::format("arrival rate: {:.3f}\n", settings.arrival_rate));

  buffer.append(std::format("service rate: {:.3f}\n", settings.service_rate));

  buffer.append(
      std::format("grade of service: {:.3f}\n",
                  blocked_count / static_cast<double>(request_count)));

  buffer.append(std::format("requests: {}\n", request_count));

  buffer.append(std::format("{}\n", Report::from(group, settings).to_string()));

  return buffer;
}
