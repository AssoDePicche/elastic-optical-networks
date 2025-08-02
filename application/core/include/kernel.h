#pragma once

#include <queue>
#include <string>
#include <unordered_set>
#include <vector>

#include "configuration.h"
#include "prng.h"
#include "request.h"
#include "spectrum.h"

enum class EventType { Arrival, Departure };

struct Event final {
  double time;
  EventType type;
  Request request;

  Event(void) = default;

  Event(const double, const EventType &, const Request &);

  [[nodiscard]] bool operator<(const Event &) const noexcept;
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
  Vertex source;
  Vertex destination;
  uint64_t FSUs;
  bool accepted;
};

class Kernel final {
  Router router;
  Carriers carriers;
  std::priority_queue<Event> queue;
  std::vector<Statistics> snapshots;
  Statistics statistics;
  std::vector<std::string> requestsKeys;
  double k_to_ignore;
  bool ignored_first_k;
  std::shared_ptr<Configuration> configuration;
  std::shared_ptr<PseudoRandomNumberGenerator> prng;

  [[nodiscard]] uint64_t GenerateKeys(const Vertex, const Vertex) const;

  [[nodiscard]] std::unordered_set<uint64_t> GenerateKeys(const Route &) const;

  [[nodiscard]] bool Dispatch(Request &);

  void Release(Request &);

  void ScheduleNextArrival(void);

  void ScheduleNextDeparture(const Event &);

 public:
  Kernel(std::shared_ptr<Configuration>);

  bool HasNext(void) const;

  void Next(void);

  [[nodiscard]] std::shared_ptr<PseudoRandomNumberGenerator>
  GetPseudoRandomNumberGenerator(void) const;

  [[nodiscard]] std::vector<Statistics> GetSnapshots(void) const;

  [[nodiscard]] Statistics GetStatistics(void) const;

  void Reset(void);
};
