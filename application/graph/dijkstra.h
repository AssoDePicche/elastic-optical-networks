#pragma once

#include "graph.h"
#include "route.h"

namespace graph {
class Dijkstra : public RoutingStrategy {
 public:
  Dijkstra(const Graph&);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};
}  // namespace graph
