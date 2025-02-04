#pragma once

#include <map>
#include <queue>
#include <string>
#include <vector>

#include "event_queue.h"
#include "group.h"
#include "logger.h"
#include "request.h"
#include "settings.h"
#include "spectrum.h"

struct Snapshot final {
  float time;
  int bandwidth;
  bool accepted;
  std::vector<float> fragmentation;
  std::vector<float> entropy;
  float blocking;

  Snapshot(float, int, bool, std::vector<float>&, std::vector<float>&, float);

  [[nodiscard]] auto str(void) const -> std::string;
};

struct Simulation final {
  Simulation(Settings&);

  void next(void);

  bool has_next(void) const;

  double blocking(void) const;

  std::vector<float> entropy(void) const;

  std::vector<float> fragmentation(void) const;

  std::vector<Snapshot> get_snapshots(void) const;

  Settings settings;
  Group group;
  EventQueue<Request> queue;
  std::map<unsigned, Spectrum> hashmap;
  std::vector<Snapshot> snapshots{};
  double time{0.0f};
  unsigned active_requests{0u};
  double accepted_requests{0.0};
  double blocked_requests{0.0};
};
