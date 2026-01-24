#pragma once

#include <memory>
#include <unordered_map>

#include "graph.h"
#include "route.h"

namespace graph {
class Router final {
 public:
  void SetStrategy(std::shared_ptr<RoutingStrategy>);

  [[nodiscard]] std::optional<Route> compute(const Vertex, const Vertex);

 private:
  std::unordered_map<uint64_t, Route> cache;
  std::shared_ptr<RoutingStrategy> strategy;
};
}  // namespace graph
