#include "graph.h"

#include <algorithm>
#include <cassert>
#include <fstream>
#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <stack>
#include <utility>

#include "distribution.h"

int operator<=>(const edge_t &u, const edge_t &v) {
  if (u.source != v.source) {
    return u.source < v.source ? -1 : 1;
  }

  if (u.destination != v.destination) {
    return u.destination < v.destination ? -1 : 1;
  }

  if (u.cost == v.cost) {
    return 0;
  }

  return u.cost < v.cost ? -1 : 1;
}

vertex_t route_t::source(void) const { return *vertices.begin(); }

vertex_t route_t::destination(void) const {
  return *vertices.begin() + vertices.size() - 1;
}

route_t route_t::none(void) { return {{}, Cost::max, __MAX_HOPS__}; }

Graph::Graph(const unsigned vertices) {
  for (auto vertex{0u}; vertex < vertices; ++vertex) {
    add(vertex);
  }
}

auto Graph::from(const std::string &filename) noexcept -> std::optional<Graph> {
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

    for (auto destination{0u}; destination < size; ++destination) {
      std::getline(stream, buffer, ' ');

      const auto cost = static_cast<cost_t>(atof(buffer.c_str()));

      if (cost != Cost::min) {
        graph.add(source, destination, cost);
      }
    }

    ++source;
  }

  return graph;
}

auto Graph::size(void) const noexcept -> unsigned { return vertices.size(); }

auto Graph::to_string(void) const noexcept -> std::string {
  std::string buffer{};

  auto matrix = std::vector(size(), std::vector(size(), Cost::min));

  for (const auto &[vertex, edges] : adjacency_list) {
    for (auto &[adjacent, cost] : edges) {
      matrix[vertex][adjacent] = cost;
    }
  }

  for (auto row{0u}; row < size(); ++row) {
    for (auto column{0u}; column < size(); ++column) {
      buffer.append(std::to_string(matrix[row][column]) + " ");
    }

    buffer.append("\n");
  }

  return buffer;
}

auto Graph::at(const vertex_t source,
               const vertex_t destination) const -> cost_t {
  for (const auto &[vertex, cost] : at(source)) {
    if (vertex == destination) {
      return cost;
    }
  }

  return Cost::min;
}

auto Graph::at(const vertex_t vertex) const -> std::list<adjacent_t> {
  return adjacency_list.at(vertex);
}

auto Graph::is_adjacent(const vertex_t source,
                        const vertex_t destination) const -> bool {
  return at(source, destination) != Cost::min;
}

auto Graph::get_vertices(void) const noexcept -> std::set<vertex_t> {
  return vertices;
}

auto Graph::get_edges(void) const noexcept -> std::set<edge_t> {
  std::set<edge_t> edges;

  for (const auto &[source, adjacent_edges] : adjacency_list) {
    for (const auto &[destination, cost] : adjacent_edges) {
      edges.insert({cost, source, destination});
    }
  }

  return edges;
}

auto Graph::add(const vertex_t vertex) -> void {
  vertices.insert(vertex);

  adjacency_list[vertex] = {};
}

auto Graph::add(const vertex_t source, const vertex_t destination,
                const cost_t cost) -> void {
  adjacency_list[source].emplace_back(destination, cost);
}

auto operator<<(std::ostream &stream, const Graph &graph) -> std::ostream & {
  return stream << graph.to_string();
}

auto breadth_first_search(const Graph &graph, const vertex_t source,
                          const vertex_t destination) noexcept -> route_t {
  assert(source != destination);

  std::unordered_map<vertex_t, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::queue<vertex_t> queue;

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

    for (const auto &[adjacent, cost] : graph.at(vertex)) {
      if (visited[adjacent]) {
        continue;
      }

      visited[adjacent] = true;

      predecessors[adjacent] = vertex;

      queue.push(adjacent);
    }
  }

  std::unordered_set<vertex_t> vertices;

  cost_t cost = Cost::min;

  for (int vertex = destination; vertex != -1; vertex = predecessors[vertex]) {
    vertices.insert(vertex);
  }

  if (*vertices.begin() != source) {
    return route_t::none();
  }

  const unsigned hops = vertices.size() - 2u;

  return {vertices, cost, hops};
}

auto depth_first_search(const Graph &graph, const vertex_t source,
                        const vertex_t destination) noexcept -> route_t {
  assert(source != destination);

  std::unordered_map<vertex_t, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::stack<vertex_t> stack;

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

    for (const auto &[adjacent, cost] : graph.at(vertex)) {
      if (visited[adjacent]) {
        continue;
      }

      predecessors[adjacent] = vertex;

      stack.push(adjacent);
    }
  }

  std::unordered_set<vertex_t> vertices;

  cost_t cost = Cost::min;

  for (int vertex = destination; vertex != -1; vertex = predecessors[vertex]) {
    vertices.insert(vertex);
  }

  if (*vertices.begin() != source) {
    return route_t::none();
  }

  const unsigned hops = vertices.size() - 2u;

  return {vertices, cost, hops};
}

auto dijkstra(const Graph &graph, const vertex_t source,
              const vertex_t destination) noexcept -> route_t {
  assert(source != destination);

  std::unordered_map<int, cost_t> costs;

  std::unordered_map<int, int> predecessors;

  std::unordered_map<int, int> edge_hops;

  // {cost, hops, vertex}
  using path_t = std::tuple<cost_t, int, int>;

  std::priority_queue<path_t, std::vector<path_t>, std::greater<>> queue;

  for (const auto &vertex : graph.get_vertices()) {
    costs[vertex] = Cost::max;

    predecessors[vertex] = -1;

    edge_hops[vertex] = __MAX_HOPS__;
  }

  costs[source] = Cost::min;

  edge_hops[source] = __MIN_HOPS__;

  queue.emplace(costs[source], edge_hops[source], source);

  while (!queue.empty()) {
    const auto &[current_cost, hops, vertex] = queue.top();

    queue.pop();

    if (vertex == static_cast<int>(destination)) {
      break;
    }

    if (current_cost > costs[vertex]) {
      continue;
    }

    if (current_cost == costs[vertex] && hops > edge_hops[vertex]) {
      continue;
    }

    for (const auto &[adjacent, cost] : graph.at(vertex)) {
      const auto new_cost{current_cost + cost};

      if (new_cost > costs[adjacent]) {
        continue;
      }

      const auto new_hops{hops + 1};

      const auto less_hops{new_hops < edge_hops[adjacent]};

      if (new_cost == costs[adjacent] && !less_hops) {
        continue;
      }

      costs[adjacent] = new_cost;

      edge_hops[adjacent] = new_hops;

      predecessors[adjacent] = vertex;

      queue.emplace(new_cost, new_hops, adjacent);
    }
  }

  std::unordered_set<vertex_t> vertices;

  cost_t cost = costs[predecessors[destination]];

  for (int vertex = destination; vertex != -1; vertex = predecessors[vertex]) {
    vertices.insert(static_cast<vertex_t>(vertex));

    cost += costs[predecessors[vertex]];
  }

  if (*vertices.begin() != source) {
    return route_t::none();
  }

  const unsigned hops = vertices.size() - 2u;

  return {vertices, cost, hops};
}

auto k_shortest_path(const Graph &graph, const vertex_t source,
                     const vertex_t destination,
                     const unsigned k) noexcept -> std::vector<route_t> {
  assert(source != destination);

  std::vector<route_t> k_paths;

  k_paths.reserve(k);

  struct stub_t {
    vertex_t vertex;
    route_t route;

    stub_t(const vertex_t vertex, const route_t &route)
        : vertex{vertex}, route{route} {}

    auto operator>(const stub_t &binding) const -> bool {
      return route.cost > binding.route.cost;
    }
  };

  std::priority_queue<stub_t, std::vector<stub_t>, std::greater<stub_t>> queue;

  queue.push(stub_t(source, {{source}, Cost::min, __MIN_HOPS__}));

  while (!queue.empty() && k_paths.size() != k) {
    auto [vertex, path] = queue.top();

    queue.pop();

    if (vertex == destination) {
      k_paths.emplace_back(path);

      continue;
    }

    for (const auto &[adjacent, cost] : graph.at(vertex)) {
      path.vertices.insert(adjacent);

      const unsigned hops = path.vertices.size() - 2u;

      queue.push(stub_t(adjacent, {path.vertices, path.cost + cost, hops}));
    }
  }

  return k_paths;
}

auto random_path(const Graph &graph) noexcept -> route_t {
  std::random_device random_device;

  Uniform distribution{random_device(), 0, static_cast<double>(graph.size())};

  auto source{static_cast<vertex_t>(distribution.next())};

  auto destination{static_cast<vertex_t>(distribution.next())};

  while (true) {
    destination = static_cast<vertex_t>(distribution.next());

    if (source == destination) {
      continue;
    }

    const auto &route = dijkstra(graph, source, destination);

    if (!route.vertices.empty()) {
      return route;
    }
  }
}
