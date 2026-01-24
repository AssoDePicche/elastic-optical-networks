#include "router.h"

#include <hash/cantor.h>

namespace graph {
void Router::SetStrategy(std::shared_ptr<RoutingStrategy> strategy) {
  this->strategy = strategy;
}

std::optional<Route> Router::compute(const Vertex source,
                                     const Vertex destination) {
  if (!strategy) {
    return std::nullopt;
  }

  const auto key = hash::CantorPairingFunction(source, destination);

  const auto iterator = cache.find(key);

  if (iterator != cache.end()) {
    return iterator->second;
  }

  const auto route = strategy->compute(source, destination);

  if (!route.has_value()) {
    return std::nullopt;
  }

  cache[key] = route.value();

  return route;
}

}  // namespace graph
