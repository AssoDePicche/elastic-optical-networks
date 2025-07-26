#pragma once

#include <optional>
#include <queue>
#include <string>
#include <vector>

#include "logger.h"
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

class EventQueue {
 public:
  [[nodiscard]] EventQueue &push(const Request &, const double);

  void of_type(const EventType &);

  [[nodiscard]] std::optional<Event> top(void) const;

  [[nodiscard]] std::optional<Event> pop(void);

  [[nodiscard]] bool empty(void) const;

  [[nodiscard]] size_t size(void) const;

 private:
  std::priority_queue<Event> queue;
  double time;
  Request to_push;
  bool pushing{false};
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

class Simulation final {
 public:
  Simulation(Settings &);

  bool HasNext(void) const;

  void Next(void);

  std::vector<Snapshot> GetSnapshots(void) const;

  double GetTime(void) const;

  double GetRequestCount(void) const;

  double GetGradeOfService(void) const;

  std::vector<double> GetFragmentation(void) const;

  void Reset(void);

 private:
  Settings &settings;
  EventQueue queue;
  Logger _logger;
  double kToIgnore;
  Dispatcher dispatcher;
  std::vector<std::string> requestsKeys;
  Router router;
  bool ignoredFirst{false};
  unsigned requestCount{0u};
  unsigned blockedCount{0u};
  unsigned activeRequests{0u};
  double time{0.0};
  std::vector<Snapshot> snapshots;
};
