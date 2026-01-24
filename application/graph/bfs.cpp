#include "bfs.h"

#include <queue>

namespace graph {
BreadthFirstSearch::BreadthFirstSearch(const Graph& graph)
    : RoutingStrategy(graph) {}

std::optional<Route> BreadthFirstSearch::compute(
    const Vertex source, const Vertex destination) const {
  std::unordered_map<Vertex, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::queue<Vertex> queue;

  for (const auto& vertex : graph.get_vertices()) {
    visited[vertex] = false;

    predecessors[vertex] = -1;
  }

  visited[source] = true;

  queue.push(source);

  while (!queue.empty()) {
    const auto vertex = queue.front();

    queue.pop();

    if (vertex == destination) {
      break;
    }

    for (const auto& [adjacent, edges] : graph.at(vertex)) {
      if (visited[adjacent]) {
        continue;
      }

      visited[adjacent] = true;

      predecessors[adjacent] = vertex;

      queue.push(adjacent);
    }
  }

  std::unordered_set<Vertex> vertices;

  Cost cost = Cost::min();

  for (int vertex = destination; vertex != -1; vertex = predecessors[vertex]) {
    vertices.insert(vertex);
  }

  if (*vertices.begin() != source) {
    return std::nullopt;
  }

  return std::make_pair(vertices, cost);
}

}  // namespace graph
