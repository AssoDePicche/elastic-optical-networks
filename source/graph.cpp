#include "graph.h"

#include <algorithm>
#include <cassert>
#include <format>
#include <fstream>
#include <limits>
#include <memory>
#include <queue>
#include <random>
#include <ranges>
#include <sstream>
#include <stack>

#include "distribution.h"

Cost::Cost(double value) : value{value} {}

Cost Cost::max(void) { return Cost(std::numeric_limits<double>::max()); }

Cost Cost::min(void) { return Cost(0.0); }

bool operator<(const Cost &lhs, const Cost &rhs) {
  return lhs.value < rhs.value;
}

Route Route::None(void) { return {{}, Cost::max()}; }

Graph::Graph(const unsigned vertices) {
  for (const auto &vertex : std::views::iota(0u, vertices)) {
    add(vertex);
  }
}

std::optional<Graph> Graph::from(const std::string &filename) noexcept {
  std::ifstream file{filename};

  if (!file.is_open()) {
    return std::nullopt;
  }

  std::string line{};

  std::getline(file, line);

  const auto size{static_cast<unsigned>(atoi(line.c_str()))};

  Graph graph{size};

  auto source{0u};

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    for (const auto &destination : std::views::iota(0u, size)) {
      std::getline(stream, buffer, ' ');

      const auto cost = static_cast<double>(atof(buffer.c_str()));

      if (Cost::min().value != cost) {
        graph.add({source, destination, cost});
      }
    }

    ++source;
  }

  return graph;
}

unsigned Graph::size(void) const noexcept { return vertices.size(); }

std::string Graph::Serialize(void) const noexcept {
  std::string buffer{};

  auto matrix = std::vector(size(), std::vector(size(), Cost::min()));

  for (const auto &[vertex, edges] : adjacency_list) {
    for (auto &[adjacent, cost] : edges) {
      matrix[vertex][adjacent] = cost.value;
    }
  }

  for (const auto &row : std::views::iota(0u, size())) {
    for (const auto &column : std::views::iota(0u, size())) {
      buffer.append(std::format("{} ", matrix[row][column].value));
    }

    buffer.append("\n");
  }

  return buffer;
}

Cost Graph::at(const Vertex source, const Vertex destination) const {
  for (const auto &[vertex, cost] : adjacency_list.at(source)) {
    if (vertex == destination) {
      return cost;
    }
  }

  return Cost::min();
}

std::list<AdjacentVertex> Graph::at(const Vertex vertex) const {
  return adjacency_list.at(vertex);
}

bool Graph::is_adjacent(const Vertex source, const Vertex destination) const {
  return Cost::min().value != at(source, destination).value;
}

std::set<Vertex> Graph::get_vertices(void) const noexcept { return vertices; }

std::vector<Edge> Graph::get_edges(void) const noexcept {
  std::vector<Edge> edges;

  for (const auto &[source, adjacent_edges] : adjacency_list) {
    for (const auto &[destination, cost] : adjacent_edges) {
      edges.push_back({source, destination, cost});
    }
  }

  return edges;
}

void Graph::add(const Vertex vertex) { vertices.insert(vertex); }

void Graph::add(const Edge &edge) {
  const auto &[source, destination, cost] = edge;

  adjacency_list[source].emplace_back(destination, cost);
}

Route BreadthFirstSearch(const Graph &graph, const Vertex source,
                         const Vertex destination) noexcept {
  assert(source != destination);

  std::unordered_map<Vertex, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::queue<Vertex> queue;

  for (const auto &vertex : graph.get_vertices()) {
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

    for (const auto &[adjacent, edges] : graph.at(vertex)) {
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
    return {{}, Cost::max()};
  }

  return {vertices, cost};
}

Route DepthFirstSearch(const Graph &graph, const Vertex source,
                       const Vertex destination) noexcept {
  assert(source != destination);

  std::unordered_map<Vertex, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::stack<Vertex> stack;

  for (const auto &vertex : graph.get_vertices()) {
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

    for (const auto &[adjacent, edges] : graph.at(vertex)) {
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
    return Route::None();
  }

  return {vertices, cost};
}

Route Dijkstra(const Graph &graph, const Vertex source,
               const Vertex destination) noexcept {
  assert(source != destination);

  std::unordered_map<int, Cost> costs;

  std::unordered_map<int, int> predecessors;

  std::unordered_map<int, int> edge_hops;

  // {cost, hops, vertex}
  using path_t = std::tuple<Cost, int, int>;

  std::priority_queue<path_t, std::vector<path_t>, std::greater<>> queue;

  for (const auto &vertex : graph.get_vertices()) {
    costs[vertex] = Cost::max();

    predecessors[vertex] = -1;

    edge_hops[vertex] = __MAX_HOPS__;
  }

  costs[source] = Cost::min();

  edge_hops[source] = __MIN_HOPS__;

  queue.emplace(costs[source], edge_hops[source], source);

  while (!queue.empty()) {
    const auto &[current_cost, hops, vertex] = queue.top();

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

    for (const auto &[adjacent, cost] : graph.at(vertex)) {
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
    return Route::None();
  }

  return {vertices, cost};
}

std::vector<Route> KShortestPath(const Graph &graph, const Vertex source,
                                 const Vertex destination,
                                 const unsigned k) noexcept {
  assert(source != destination);

  std::vector<Route> kShortestPaths{};

  struct stub_t {
    Vertex vertex;
    Route path;

    stub_t(const Vertex vertex, const Route &path)
        : vertex{vertex}, path{path} {}

    auto operator>(const stub_t &binding) const -> bool {
      const auto &[vertices, cost] = path;

      const auto &[other_vertices, other_cost] = binding.path;

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

    for (const auto &[adjacent, cost] : graph.at(vertex)) {
      auto &[vertices, path_cost] = path;

      vertices.insert(adjacent);

      queue.push(stub_t(adjacent, {vertices, path_cost.value + cost.value}));
    }
  }

  return kShortestPaths;
}

Route random_path(const Graph &graph) noexcept {
  static std::random_device random_device;

  static Uniform distribution{random_device(), 0,
                              static_cast<double>(graph.size())};

  const auto source{static_cast<Vertex>(distribution.next())};

  auto destination{static_cast<Vertex>(distribution.next())};

  while (true) {
    destination = static_cast<Vertex>(distribution.next());

    if (source == destination) {
      continue;
    }

    const auto &[vertices, cost] = Dijkstra(graph, source, destination);

    if (!vertices.empty()) {
      return {vertices, cost};
    }
  }
}
