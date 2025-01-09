#pragma once

#include <string>

#include "settings.h"

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

[[nodiscard]] auto simulation(Settings &) -> std::string;
