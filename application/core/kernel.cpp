#include "kernel.h"

#include <hash/cantor.h>

#include <format>

namespace core {
Event::Event(const double time, const EventType& type, const Request& request)
    : time{time}, type{type}, request{request} {}

bool Event::operator<(const Event& other) const noexcept {
  return time > other.time;
}

void Statistics::Reset(void) {
  absolute_fragmentation = 0;

  entropy_fragmentation = 0;

  external_fragmentation = 0;

  time = 0;

  active_requests = 0;

  total_FSUs_requested = 0;

  total_FSUs_blocked = 0;

  total_requests = 0;

  total_requests_blocked = 0;
}

double Statistics::GradeOfService(void) const {
  return static_cast<double>(total_requests_blocked) /
         static_cast<double>(total_requests);
}

double Statistics::SlotBlockingProbability(void) const {
  return static_cast<double>(total_FSUs_blocked) /
         static_cast<double>(total_FSUs_requested);
}

std::string Statistics::Serialize(void) const {
  return std::format("{:3},{:3},{:3},{:3},{:3},{:3},{}", time,
                     absolute_fragmentation, entropy_fragmentation,
                     external_fragmentation, GradeOfService(),
                     SlotBlockingProbability(), active_requests);
}

uint64_t Kernel::GenerateKeys(const graph::Vertex source,
                              const graph::Vertex destination) const {
  return hash::CantorPairingFunction(source, destination);
}

std::unordered_set<uint64_t> Kernel::GenerateKeys(
    const graph::Route& route) const {
  const auto& [vertices, cost] = route;

  std::unordered_set<uint64_t> keys;

  for (const auto& index : std::views::iota(1u, vertices.size())) {
    const auto x = *std::next(vertices.begin(), index - 1);

    const auto y = *std::next(vertices.begin(), index);

    keys.insert(hash::CantorPairingFunction(x, y));
  }

  return keys;
}

bool Kernel::Dispatch(Request& request) {
  const auto keys = GenerateKeys(request.route);

  const auto first = *keys.begin();

  const auto slice = request.type.allocator(carriers[first], request.type.FSUs);

  if (!slice.has_value()) {
    return false;
  }

  request.slice = slice.value();

  for (const auto& key : keys) {
    if (carriers.at(key).available() < request.type.FSUs ||
        !carriers.at(key).available_at(request.slice)) {
      return false;
    }
  }

  const auto allocate = [&](const auto key) {
    carriers[key].allocate(request.slice);
  };

  std::for_each(keys.begin(), keys.end(), allocate);

  return true;
}

void Kernel::Release(Request& request) {
  const auto keys = GenerateKeys(request.route);

  std::for_each(keys.begin(), keys.end(), [&](const auto key) {
    carriers[key].deallocate(request.slice);
  });
}

void Kernel::ScheduleNextArrival(void) {
  auto& requestType =
      configuration->requestTypes[requestsKeys[prng->Next("fsus")]];

  ++requestType.counting;

  auto request =
      Request(router.compute(graph::NullVertex, graph::NullVertex).value());

  request.type = requestType;

  queue.push(Event(statistics.time + prng->Next("arrival"), EventType::Arrival,
                   request));

  statistics.total_FSUs_requested += request.type.FSUs;

  ++statistics.total_requests;
}

void Kernel::ScheduleNextDeparture(const Event& event) {
  queue.push(Event(statistics.time + prng->Next("service"),
                   EventType::Departure, event.request));
}

Kernel::Kernel(std::shared_ptr<Configuration> configuration)
    : k_to_ignore{0.1 * configuration->timeUnits},
      configuration{configuration} {
  for (const auto& [source, destination, cost] :
       configuration->graph.get_edges()) {
    const auto key = GenerateKeys(source, destination);

    carriers[key] = Spectrum(configuration->FSUsPerLink);
  }

  Reset();
}

bool Kernel::HasNext(void) const {
  return !queue.empty() && queue.top().time <= configuration->timeUnits;
}

void Kernel::Next(void) {
  auto event = queue.top();

  queue.pop();

  statistics.time = event.time;

  if (configuration->ignoreFirst && statistics.time > k_to_ignore &&
      !ignored_first_k) {
    ignored_first_k = true;

    statistics.Reset();

    for (auto& request : configuration->requestTypes) {
      request.second.blocking = 0u;

      request.second.counting = 0u;
    }

    configuration->logger->log(Logger::Level::Info,
                               "Discard first {:.3f} time units",
                               statistics.time);
  }

  if (event.type == EventType::Departure) {
    --statistics.active_requests;

    configuration->logger->log(Logger::Level::Info,
                               "Request for {} FSU(s) departing at {:.3f}",
                               event.request.type.FSUs, event.time);

    Release(event.request);

    return;
  }

  event.request.accepted = false;

  if (statistics.active_requests < configuration->FSUsPerLink &&
      Dispatch(event.request)) {
    ++statistics.active_requests;

    configuration->logger->log(Logger::Level::Info,
                               "Accept request for {} FSU(s) at {:.3f}",
                               event.request.type.FSUs, statistics.time);

    event.request.accepted = true;

    ScheduleNextDeparture(event);
  } else {
    configuration->logger->log(Logger::Level::Info,
                               "Blocking request for {} FSU(s) at {:.3f}",
                               event.request.type.FSUs, event.time);

    statistics.total_FSUs_blocked += event.request.type.FSUs;

    for (auto& [_, requestType] : configuration->requestTypes) {
      if (event.request.type.FSUs != requestType.FSUs) {
        continue;
      }

      ++requestType.blocking;

      break;
    }

    ++statistics.total_requests_blocked;
  }

  if (snapshots.empty() ||
      abs(snapshots.back().time - event.time) >= configuration->samplingTime) {
    statistics.absolute_fragmentation = 0.0;

    statistics.entropy_fragmentation = 0.0;

    statistics.external_fragmentation = 0.0;

    const auto frag = configuration->fragmentationStrategies;

    for (const auto& [source, destination, cost] :
         configuration->graph.get_edges()) {
      const auto key = GenerateKeys(source, destination);

      statistics.absolute_fragmentation +=
          (*(frag.at("absolute_fragmentation")))(carriers.at(key));

      statistics.entropy_fragmentation +=
          (*(frag.at("entropy_based_fragmentation")))(carriers.at(key));

      statistics.external_fragmentation +=
          (*(frag.at("external_fragmentation")))(carriers.at(key));
    }

    snapshots.push_back(statistics);
  }

  ScheduleNextArrival();
}

void Kernel::Run(void) {
  while (HasNext()) {
    Next();
  }
}

void Kernel::Reset(void) {
  statistics.Reset();

  ignored_first_k = false;

  snapshots.clear();

  requestsKeys.clear();

  std::transform(configuration->requestTypes.begin(),
                 configuration->requestTypes.end(),
                 std::back_inserter(requestsKeys), [](auto& pair) {
                   pair.second.blocking = 0;

                   pair.second.counting = 0;

                   return pair.first;
                 });

  prng = prng::PseudoRandomNumberGenerator::Instance();

  prng->SetRandomSeed();

  prng->SetExponentialVariable("arrival", configuration->arrivalRate);

  prng->SetExponentialVariable("service", configuration->serviceRate);

  prng->SetDiscreteVariable("fsus", configuration->probs.begin(),
                            configuration->probs.end());

  prng->SetUniformVariable("routing", 0, configuration->graph.size());

  router.SetStrategy(
      std::make_shared<graph::RandomRouting>(configuration->graph));

  ScheduleNextArrival();
}

Document Kernel::GetReport(void) const {
  const auto time = std::time(nullptr);

  const auto localtime = std::localtime(&time);

  const auto kernel_time = statistics.time;

  const auto requestCount = static_cast<double>(statistics.total_requests);

  core::Document document;

  document
      .append("created at: {:02}/{:02}/{:04} {:02}h{:02}\n", localtime->tm_mday,
              localtime->tm_mon + 1, localtime->tm_year + 1900,
              localtime->tm_hour, localtime->tm_min)
      .append("seed: {}\n", prng->GetSeed())
      .append("simulated time: {:.3f}\n", kernel_time)
      .append("spectrum width (GHz): {:.2f}\n", configuration->spectrumWidth)
      .append("slot width (GHz): {:.2f}\n", configuration->slotWidth)
      .append("fsus per link: {}\n", configuration->FSUsPerLink);

  const double load = configuration->arrivalRate / configuration->serviceRate;

  document.append("load (E): {:.3f}\n", load)
      .append("arrival rate: {:.3f}\n", configuration->arrivalRate)
      .append("service rate: {:.3f}\n", configuration->serviceRate)
      .append("grade of service: {:.3f}\n", statistics.GradeOfService())
      .append("total requests: {}\n", requestCount);

  for (const auto& [_, requestType] : configuration->requestTypes) {
    const auto ratio = requestType.counting / requestCount;

    const auto gos = requestType.blocking / requestCount;

    const auto normalized_load =
        configuration->arrivalRate *
        (static_cast<double>(requestType.FSUs) / configuration->FSUsPerLink);

    document.append("requests for {} FSU(s)\n", requestType.FSUs)
        .append("ratio: {:.3f}\n", ratio)
        .append("grade of service: {:.3f}\n", gos)
        .append("normalized load: {:.3f}\n", normalized_load);
  }

  return document;
}

void Kernel::ExportDataset(const std::string& filename) const {
  std::string buffer{
      "time,absolute_fragmentation,entropy,external_fragmentation,grade_of_"
      "service,slot_blocking_probability,active_requests\n"};

  std::for_each(snapshots.begin(), snapshots.end(),
                [&buffer](const core::Statistics& snapshot) {
                  buffer.append(std::format("{}\n", snapshot.Serialize()));
                });

  std::ofstream stream(filename);

  if (!stream.is_open()) {
    throw std::runtime_error(std::format("Failed to write {} file", filename));
  }

  stream << buffer;

  stream.close();
}
}  // namespace core
