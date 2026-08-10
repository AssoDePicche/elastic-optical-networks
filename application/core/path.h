#pragma once

#include <vector>

#include "vertex.h"

namespace core {
struct Path final {
  double cost;
  std::vector<Vertex> vertices;

  [[nodiscard]] bool empty() const noexcept { return vertices.empty(); }

  [[nodiscard]] bool operator>(const Path& other) const {
    return cost > other.cost;
  }
};
};  // namespace core
