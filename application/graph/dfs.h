#pragma once

#include "graph.h"
#include "route.h"

namespace graph {
class DepthFirstSearch : public RoutingStrategy {
 public:
  DepthFirstSearch(const Graph&);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};
}  // namespace graph
