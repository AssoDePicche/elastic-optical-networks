#pragma once

#include <map>
#include <string>

#include "distribution.h"
#include "event_queue.h"
#include "request.h"
#include "settings.h"
#include "spectrum.h"

struct Snapshot final {
  float time;
  int slots;
  bool accepted;
  float fragmentation;
  float entropy;
  float blocking;

  Snapshot(float, int, bool, float, float, float);

  [[nodiscard]] auto str(void) const -> std::string;
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

 private:
  Settings settings;
  EventQueue<Request> queue;
  Discrete distribution;
  double kToIgnore;
  std::map<unsigned, Spectrum> hashmap;
  std::vector<std::string> requestsKeys;
  bool ignoredFirst{false};
  unsigned requestCount{0u};
  unsigned blockedCount{0u};
  unsigned activeRequests{0u};
  double time{0.0};
  std::vector<Snapshot> snapshots;

  double network_fragmentation(void);

  double entropy(void);
};
