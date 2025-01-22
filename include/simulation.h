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
  int slots;
  bool accepted;
  float fragmentation;
  float entropy;
  float blocking;

  Snapshot(float, int, bool, float, float, float);

  [[nodiscard]] auto str(void) const -> std::string;
};

struct Simulation final {
  Simulation(Settings&);

  void next(void);

  bool has_next(void) const;

  double entropy(void) const;

  double fragmentation(void) const;

  std::vector<Snapshot> get_snapshots(void) const;

  Settings settings;
  Group group;
  EventQueue<Request> queue;
  std::map<unsigned, Spectrum> hashmap;
  std::vector<Snapshot> snapshots{};
  double time{0.0f};
  unsigned active_calls{0u};
};
