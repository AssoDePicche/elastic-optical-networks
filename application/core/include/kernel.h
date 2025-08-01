#pragma once

#include <queue>
#include <string>
#include <vector>

#include "configuration.h"
#include "logger.h"
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

struct Snapshot final {
  double time;
  uint64_t FSUs;
  bool accepted;
  std::vector<double> fragmentation;
  double blocking;

  Snapshot(const Event &, std::vector<double>, double);

  [[nodiscard]] std::string Serialize(void) const;
};

class Kernel final {
  Router router;
  Carriers carriers;
  std::priority_queue<Event> queue;
  std::vector<Snapshot> snapshots;
  std::vector<std::string> requestsKeys;
  double k_to_ignore;
  double time;
  uint64_t requestCount;
  uint64_t blockedCount;
  uint64_t activeRequests;
  bool ignored_first_k;
  std::shared_ptr<Configuration> configuration;
  std::shared_ptr<PseudoRandomNumberGenerator> prng;

  [[nodiscard]] bool Dispatch(Request &, const SpectrumAllocator &);

  void Release(Request &);

  void ScheduleNextArrival(void);

  void ScheduleNextDeparture(const Event &);

 public:
  Kernel(std::shared_ptr<Configuration>);

  bool HasNext(void) const;

  void Next(void);

  [[nodiscard]] std::shared_ptr<PseudoRandomNumberGenerator>
  GetPseudoRandomNumberGenerator(void) const;

  [[nodiscard]] std::vector<Snapshot> GetSnapshots(void) const;

  [[nodiscard]] double GetTime(void) const;

  [[nodiscard]] double GetRequestCount(void) const;

  [[nodiscard]] double GetGradeOfService(void) const;

  [[nodiscard]] std::vector<double> GetFragmentation(void) const;

  void Reset(void);
};
