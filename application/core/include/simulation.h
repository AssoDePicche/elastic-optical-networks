#pragma once

#include <string>
#include <vector>

#include "event_queue.h"
#include "request.h"
#include "settings.h"
#include "spectrum.h"

struct Snapshot final {
  double time;
  unsigned FSUs;
  bool accepted;
  std::vector<double> fragmentation;
  double entropy;
  double blocking;

  Snapshot(const Event<Request> &, std::vector<double>, double, double);

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

  double GetEntropy(void) const;

  std::vector<double> GetFragmentation(void) const;

  void Reset(void);

 private:
  Settings &settings;
  EventQueue<Request> queue;
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
