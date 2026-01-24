#pragma once

#include <vector>

#include "graph.h"
#include "route.h"

namespace graph {
class KShortestPath {
 public:
  KShortestPath(const Graph&);

  [[nodiscard]] std::vector<Route> compute(const Vertex, const Vertex,
                                           const uint64_t) const;

 private:
  const Graph& graph;
};
}  // namespace graph
