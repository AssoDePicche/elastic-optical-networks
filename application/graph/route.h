#pragma once

#include <optional>
#include <unordered_set>

#include "graph.h"

namespace graph {
using Route = std::pair<std::unordered_set<Vertex>, Cost>;

class RoutingStrategy {
 public:
  RoutingStrategy(const Graph&);

  virtual ~RoutingStrategy() = default;

  [[nodiscard]] virtual std::optional<Route> compute(const Vertex,
                                                     const Vertex) const = 0;

 protected:
  const Graph& graph;
};

class RandomRouting : public RoutingStrategy {
 public:
  RandomRouting(const Graph&);

  [[nodiscard]] std::optional<Route> compute(const Vertex,
                                             const Vertex) const override;
};
}  // namespace graph
