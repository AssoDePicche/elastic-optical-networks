#pragma once

#include <vector>

#include "graph.h"

namespace core {
struct Path final {
  std::vector<Graph::Vertex> vertices;
  Graph::Edge::Cost cost;

  [[nodiscard]] bool empty() const noexcept;

  [[nodiscard]] std::vector<uint64_t> keys() const noexcept;

  [[nodiscard]] bool operator>(const Path&) const;
};
};  // namespace core
