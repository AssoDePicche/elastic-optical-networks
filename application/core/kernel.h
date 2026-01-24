#pragma once

#include <graph/router.h>
#include <prng/prng.h>

#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "configuration.h"
#include "document.h"
#include "request.h"
#include "spectrum.h"

namespace core {
enum class EventType { Arrival, Departure };

struct Event final {
  double time;
  EventType type;
  Request request;

  Event(void) = default;

  Event(const double, const EventType&, const Request&);

  [[nodiscard]] bool operator<(const Event&) const noexcept;
};

struct Statistics final {
  double absolute_fragmentation;
  double entropy_fragmentation;
  double external_fragmentation;
  double time;
  uint64_t active_requests;
  uint64_t total_FSUs_requested;
  uint64_t total_FSUs_blocked;
  uint64_t total_requests;
  uint64_t total_requests_blocked;

  Statistics(void) = default;

  void Reset(void);

  [[nodiscard]] double GradeOfService(void) const;

  [[nodiscard]] double SlotBlockingProbability(void) const;

  [[nodiscard]] std::string Serialize(void) const;
};

struct Trace final {
  std::string type;
  graph::Vertex source;
  graph::Vertex destination;
  uint64_t FSUs;
  bool accepted;
};

class Kernel final {
  graph::Router router;
  Carriers carriers;
  std::priority_queue<Event> queue;
  std::vector<Statistics> snapshots;
  Statistics statistics;
  std::vector<std::string> requestsKeys;
  double k_to_ignore;
  bool ignored_first_k;
  std::shared_ptr<Configuration> configuration;
  std::shared_ptr<prng::PseudoRandomNumberGenerator> prng;

  [[nodiscard]] uint64_t GenerateKeys(const graph::Vertex,
                                      const graph::Vertex) const;

  [[nodiscard]] std::unordered_set<uint64_t> GenerateKeys(
      const graph::Route&) const;

  [[nodiscard]] bool Dispatch(Request&);

  void Release(Request&);

  void ScheduleNextArrival(void);

  void ScheduleNextDeparture(const Event&);

 public:
  Kernel(std::shared_ptr<Configuration>);

  bool HasNext(void) const;

  void Next(void);

  void Run(void);

  [[nodiscard]] Document GetReport(void) const;

  void ExportDataset(const std::string&) const;

  void Reset(void);
};
}  // namespace core
