#include "dfs.h"

#include <stack>

namespace graph {
DepthFirstSearch::DepthFirstSearch(const Graph& graph)
    : RoutingStrategy{graph} {}

std::optional<Route> DepthFirstSearch::compute(const Vertex source,
                                               const Vertex destination) const {
  std::unordered_map<Vertex, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::stack<Vertex> stack;

  for (const auto& vertex : graph.get_vertices()) {
    visited[vertex] = false;

    predecessors[vertex] = -1;
  }

  stack.push(source);

  while (!stack.empty()) {
    const auto vertex = stack.top();

    stack.pop();

    if (visited[vertex]) {
      continue;
    }

    visited[vertex] = true;

    if (vertex == destination) {
      break;
    }

    for (const auto& [adjacent, edges] : graph.at(vertex)) {
      if (visited[adjacent]) {
        continue;
      }

      predecessors[adjacent] = vertex;

      stack.push(adjacent);
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
