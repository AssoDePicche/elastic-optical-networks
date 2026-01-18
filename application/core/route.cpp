#include "route.h"

#include <queue>
#include <random>
#include <stack>

#include "prng.h"
#include "request.h"

namespace core {
uint64_t CantorPairingFunction(uint64_t x, uint64_t y) {
  return ((x + y) * (x + y + 1) / 2) + y;
}

RoutingStrategy::RoutingStrategy(const Graph& graph) : graph{graph} {}

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

RandomRouting::RandomRouting(const Graph& graph) : RoutingStrategy{graph} {}

std::optional<Route> RandomRouting::compute(const Vertex, const Vertex) const {
  static Dijkstra dijkstra(graph);

  while (true) {
    auto source = static_cast<Vertex>(
        PseudoRandomNumberGenerator::Instance()->next("routing"));

    auto destination = static_cast<Vertex>(
        PseudoRandomNumberGenerator::Instance()->next("routing"));

    if (source == destination) {
      continue;
    }

    const auto route = dijkstra.compute(source, destination);

    if (route.has_value()) {
      return route;
    }
  }
}

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

void Router::SetStrategy(std::shared_ptr<RoutingStrategy> strategy) {
  this->strategy = strategy;
}

std::optional<Route> Router::compute(const Vertex source,
                                     const Vertex destination) {
  if (!strategy) {
    return std::nullopt;
  }

  const auto key = CantorPairingFunction(source, destination);

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
}  // namespace core
