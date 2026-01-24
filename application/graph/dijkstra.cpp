#include "dijkstra.h"

#include <queue>

namespace graph {
Dijkstra::Dijkstra(const Graph& graph) : RoutingStrategy{graph} {}

std::optional<Route> Dijkstra::compute(const Vertex source,
                                       const Vertex destination) const {
  std::unordered_map<int, Cost> costs;

  std::unordered_map<int, int> predecessors;

  std::unordered_map<int, int> edge_hops;

  // {cost, hops, vertex}
  using path_t = std::tuple<Cost, int, int>;

  std::priority_queue<path_t, std::vector<path_t>, std::greater<>> queue;

  for (const auto& vertex : graph.get_vertices()) {
    costs[vertex] = Cost::max();

    predecessors[vertex] = -1;

    edge_hops[vertex] = static_cast<long long int>(NullVertex);
  }

  costs[source] = Cost::min();

  edge_hops[source] = 0u;

  queue.emplace(costs[source], edge_hops[source], source);

  while (!queue.empty()) {
    const auto& [current_cost, hops, vertex] = queue.top();

    queue.pop();

    if (vertex == static_cast<int>(destination)) {
      break;
    }

    if (current_cost.value > costs[vertex].value) {
      continue;
    }

    if (current_cost.value == costs[vertex].value && hops > edge_hops[vertex]) {
      continue;
    }

    for (const auto& [adjacent, cost] : graph.at(vertex)) {
      const auto new_cost = Cost(current_cost.value + cost.value);

      if (new_cost.value > costs[adjacent].value) {
        continue;
      }

      const auto new_hops{hops + 1};

      const auto less_hops{new_hops < edge_hops[adjacent]};

      if (new_cost.value == costs[adjacent].value && !less_hops) {
        continue;
      }

      costs[adjacent] = new_cost;

      edge_hops[adjacent] = new_hops;

      predecessors[adjacent] = vertex;

      queue.emplace(new_cost, new_hops, adjacent);
    }
  }

  std::unordered_set<Vertex> vertices;

  Cost cost = Cost::min();

  for (int vertex = destination; vertex != -1; vertex = predecessors[vertex]) {
    vertices.insert(static_cast<Vertex>(vertex));

    cost.value += costs[vertex].value;
  }

  if (*vertices.begin() != source) {
    return std::nullopt;
  }

  return std::make_pair(vertices, cost);
}
}  // namespace graph
