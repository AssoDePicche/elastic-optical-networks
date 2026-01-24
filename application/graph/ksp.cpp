#include "ksp.h"

#include <queue>

namespace graph {
KShortestPath::KShortestPath(const Graph& graph) : graph{graph} {}

std::vector<Route> KShortestPath::compute(const Vertex source,
                                          Vertex destination,
                                          const uint64_t k) const {
  std::vector<Route> kShortestPaths{};

  struct stub_t {
    Vertex vertex;
    Route path;

    stub_t(const Vertex vertex, const Route& path)
        : vertex{vertex}, path{path} {}

    bool operator>(const stub_t& binding) const {
      const auto& [vertices, cost] = path;

      const auto& [other_vertices, other_cost] = binding.path;

      return cost.value > other_cost.value;
    }
  };

  std::priority_queue<stub_t, std::vector<stub_t>, std::greater<stub_t>> queue;

  queue.push(stub_t(source, {{source}, Cost::min()}));

  while (!queue.empty() && kShortestPaths.size() != k) {
    auto [vertex, path] = queue.top();

    queue.pop();

    if (vertex == destination) {
      kShortestPaths.push_back(path);

      continue;
    }

    for (const auto& [adjacent, cost] : graph.at(vertex)) {
      auto& [vertices, path_cost] = path;

      vertices.insert(adjacent);

      queue.push(stub_t(adjacent, {vertices, path_cost.value + cost.value}));
    }
  }

  return kShortestPaths;
}
}  // namespace graph
