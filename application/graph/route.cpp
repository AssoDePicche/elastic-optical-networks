#include "route.h"

#include <prng/prng.h>

#include "dijkstra.h"

namespace graph {
RoutingStrategy::RoutingStrategy(const Graph& graph) : graph{graph} {}

RandomRouting::RandomRouting(const Graph& graph) : RoutingStrategy{graph} {}

std::optional<Route> RandomRouting::compute(const Vertex, const Vertex) const {
  static Dijkstra dijkstra(graph);

  while (true) {
    auto source = static_cast<Vertex>(
        prng::PseudoRandomNumberGenerator::Instance()->Next("routing"));

    auto destination = static_cast<Vertex>(
        prng::PseudoRandomNumberGenerator::Instance()->Next("routing"));

    if (source == destination) {
      continue;
    }

    const auto route = dijkstra.compute(source, destination);

    if (route.has_value()) {
      return route;
    }
  }
}
}  // namespace graph
