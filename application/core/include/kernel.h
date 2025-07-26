#pragma once

#include <queue>
#include <string>
#include <vector>

#include "logger.h"
#include "prng.h"
#include "request.h"
#include "settings.h"
#include "spectrum.h"

enum class EventType { Arrival, Departure };

struct Event {
  double time;
  EventType type;
  Request request;

  Event(void) = default;

  Event(const double, const EventType &, const Request &);

  [[nodiscard]] bool operator<(const Event &) const noexcept;
};

struct Snapshot final {
  double time;
  unsigned FSUs;
  bool accepted;
  std::vector<double> fragmentation;
  double blocking;

  Snapshot(const Event &, std::vector<double>, double);

  [[nodiscard]] std::string Serialize(void) const;
};

class Kernel final {
  Settings &settings;
  Dispatcher dispatcher;
  Router router;
  std::priority_queue<Event> queue;
  std::vector<Snapshot> snapshots;
  std::vector<std::string> requestsKeys;
  double kToIgnore;
  double time;
  unsigned requestCount;
  unsigned blockedCount;
  unsigned activeRequests;
  bool ignoredFirst;
  std::shared_ptr<Logger> logger;
  std::shared_ptr<PseudoRandomNumberGenerator> prng;

 public:
  Kernel(Settings &);

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
