#include "simulation.h"

#include "event_queue.h"
#include "graph.h"
#include "group.h"
#include "logger.h"
#include "request.h"
#include "statistics.h"
#include "timer.h"

Snapshot::Snapshot(float time, int bandwidth, bool accepted,
                   std::vector<float>& fragmentation,
                   std::vector<float>& entropy, float blocking)
    : time{time},
      bandwidth{bandwidth},
      accepted{accepted},
      fragmentation{fragmentation},
      entropy{entropy},
      blocking{blocking} {}

auto Snapshot::str(void) const -> std::string {
  std::string string = std::to_string(time) + ',' + std::to_string(bandwidth) +
                       ',' + (accepted ? "True" : "False") + ',';

  for (const auto& f : fragmentation) {
    string += std::to_string(f) + ',';
  }

  for (const auto& e : entropy) {
    string += std::to_string(e) + ',';
  }

  return string + std::to_string(blocking);
}

Simulation::Simulation(Settings& settings)
    : settings{settings},
      group{settings.seed, {50.0, 50.0f}, {3, 5}},
      queue{settings.arrival_rate, settings.service_rate, settings.seed},
      hashmap{make_hashmap(settings.graph, settings.bandwidth)} {
  queue.push(Request{random_path(settings.graph), group.next()}, time)
      .of_type(Signal::ARRIVAL);
}

void Simulation::next(void) {
  const auto top{queue.pop()};

  auto [now, signal, request] = top.value();

  time = now;

  INFO("Now: " + std::to_string(now));

  if (signal == Signal::DEPARTURE) {
    --active_requests;

    INFO("Request for " + std::to_string(request.bandwidth) +
         " bandwidth slots departing");

    const auto keys{route_keys(request.route)};

    for (const auto& key : keys) {
      hashmap[key].deallocate(request.slice);

      INFO(hashmap[key].to_string());
    }

    return;
  }

  const auto allocator = (request.bandwidth == 3) ? first_fit : last_fit;

  auto accepted = false;

  if (active_requests < settings.bandwidth &&
      dispatch_request(request, hashmap, allocator)) {
    ++active_requests;

    ++accepted_requests;

    queue.push(request, now).of_type(Signal::DEPARTURE);

    INFO("Accept request for " + std::to_string(request.bandwidth) +
         " bandwidth slots");

    const auto keys{route_keys(request.route)};

    for (const auto& key : keys) {
      INFO(hashmap[key].to_string());
    }

    accepted = true;

  } else {
    ++blocked_requests;

    INFO("Blocking request for " + std::to_string(request.bandwidth) +
         " bandwidth slots");

    group.count_blocking(request.bandwidth);
  }

  auto f = fragmentation();

  auto e = entropy();

  snapshots.push_back(
      Snapshot(now, request.bandwidth, accepted, f, e, blocking()));

  queue.push(Request{random_path(settings.graph), group.next()}, now)
      .of_type(Signal::ARRIVAL);
}

bool Simulation::has_next(void) const {
  return queue.top().value().time <= settings.time_units;
}

double Simulation::blocking(void) const {
  return accepted_requests / (blocked_requests + accepted_requests);
}

std::vector<float> Simulation::entropy(void) const {
  std::vector<float> link_entropy;

  for (const auto& [source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    link_entropy.push_back(shannon_entropy(hashmap.at(key)));
  }

  return link_entropy;
}

std::vector<float> Simulation::fragmentation(void) const {
  std::vector<float> link_fragmentation;

  for (const auto& [source, destination, cost] : settings.graph.get_edges()) {
    const auto key = make_key(source, destination);

    link_fragmentation.push_back(hashmap.at(key).fragmentation());
  }

  return link_fragmentation;
}

std::vector<Snapshot> Simulation::get_snapshots(void) const {
  return snapshots;
}
