#include "graph.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <queue>
#include <random>
#include <sstream>
#include <stack>
#include <utility>

#include "distribution.h"

Path::Path(const std::vector<Vertex> &vertices, const Cost cost)
    : vertices{vertices}, cost{cost} {}

auto Path::source(void) const -> Vertex { return vertices.front(); }

auto Path::destination(void) const -> Vertex { return vertices.back(); }

auto Path::operator>(const Path &path) const -> bool {
  return cost > path.cost;
}

Graph::Graph(const std::size_t vertices) {
  for (auto vertex{0u}; vertex < vertices; ++vertex) {
    add_vertex(vertex);
  }
}

auto Graph::from(const std::string &filename) noexcept -> std::optional<Graph> {
  std::ifstream file{filename};

  if (!file.is_open()) {
    return std::nullopt;
  }

  std::string line{};

  std::getline(file, line);

  const auto size{static_cast<std::size_t>(atoi(line.c_str()))};

  Graph graph{size};

  auto source{0u};

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    for (auto destination{0u}; destination < size; ++destination) {
      std::getline(stream, buffer, ' ');

      const auto cost = static_cast<Cost>(atof(buffer.c_str()));

      if (cost != __MIN_COST__) {
        graph.add_edge(source, destination, cost);
      }
    }

    ++source;
  }

  return graph;
}

auto Graph::size(void) const noexcept -> std::size_t { return vertices.size(); }

auto Graph::to_string(void) const noexcept -> std::string {
  std::string buffer{};

  auto matrix = std::vector(size(), std::vector(size(), __MIN_COST__));

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

auto Graph::breadth_first_search(const Vertex source,
                                 const Vertex destination) noexcept -> Path {
  if (source == destination) {
    return Path({}, __MAX_COST__);
  }

  std::unordered_map<Vertex, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::queue<Vertex> queue;

  for (const auto &[id, vertex] : vertices) {
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

    for (const auto &[adjacent, edges] : adjacency_list[vertex]) {
      if (visited[adjacent]) {
        continue;
      }

      visited[adjacent] = true;

      predecessors[adjacent] = vertex;

      queue.push(adjacent);
    }
  }

  std::vector<Vertex> vertices{};

  auto vertex = static_cast<int>(destination);

  while (vertex != -1) {
    vertices.push_back(vertex);

    vertex = predecessors[vertex];
  }

  std::reverse(vertices.begin(), vertices.end());

  if (vertices.front() != source) {
    vertices.clear();
  }

  return Path(vertices, __MAX_COST__);
}

auto Graph::depth_first_search(const Vertex source,
                               const Vertex destination) noexcept -> Path {
  if (source == destination) {
    return Path({}, __MAX_COST__);
  }

  std::unordered_map<Vertex, bool> visited;

  std::unordered_map<int, int> predecessors;

  std::stack<Vertex> stack;

  for (const auto &[id, vertex] : vertices) {
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

    for (const auto &[adjacent, edges] : adjacency_list[vertex]) {
      if (visited[adjacent]) {
        continue;
      }

      predecessors[adjacent] = vertex;

      stack.push(adjacent);
    }
  }

  std::vector<Vertex> vertices{};

  auto vertex = static_cast<int>(destination);

  while (vertex != -1) {
    vertices.push_back(vertex);

    vertex = predecessors[vertex];
  }

  std::reverse(vertices.begin(), vertices.end());

  if (vertices.front() != source) {
    vertices.clear();
  }

  return Path(vertices, __MAX_COST__);
}

auto Graph::dijkstra(const Vertex source, const Vertex destination) noexcept
    -> Path {
  if (source == destination) {
    return Path({}, __MAX_COST__);
  }

  std::unordered_map<int, Cost> costs;

  std::unordered_map<int, int> predecessors;

  std::unordered_map<int, int> edge_hops;

  // {cost, {hops, vertex}}
  using PathInfo = std::pair<Cost, std::pair<int, int>>;

  std::priority_queue<PathInfo, std::vector<PathInfo>, std::greater<>> queue;

  for (const auto &[id, name] : vertices) {
    costs[id] = __MAX_COST__;

    predecessors[id] = -1;

    edge_hops[id] = __MAX_HOPS__;
  }

  costs[source] = __MIN_COST__;

  edge_hops[source] = __MIN_HOPS__;

  queue.emplace(costs[source], std::make_pair(edge_hops[source], source));

  while (!queue.empty()) {
    const auto current_cost = queue.top().first;

    const auto hops = queue.top().second.first;

    const auto vertex = queue.top().second.second;

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

    for (const auto &[adjacent, cost] : adjacency_list[vertex]) {
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

      queue.emplace(new_cost, std::make_pair(new_hops, adjacent));
    }
  }

  std::vector<Vertex> vertices{};

  auto current = static_cast<int>(destination);

  while (current != -1) {
    vertices.push_back(static_cast<std::size_t>(current));

    current = predecessors[current];
  }

  std::reverse(vertices.begin(), vertices.end());

  if (vertices.front() != source) {
    vertices.clear();
  }

  const Cost cost = __MAX_COST__;

  return Path(vertices, cost);
}

auto Graph::k_shortest_path(const Vertex source, const Vertex destination,
                            const unsigned k) noexcept -> std::vector<Path> {
  if (source == destination) {
    return {};
  }

  std::vector<Path> k_paths{};

  struct Binding {
    Vertex vertex;
    Path path;

    Binding(const Vertex vertex, const Path &path)
        : vertex{vertex}, path{path} {}

    auto operator>(const Binding &binding) const -> bool {
      return path.cost > binding.path.cost;
    }
  };

  std::priority_queue<Binding, std::vector<Binding>, std::greater<Binding>>
      queue;

  queue.push(Binding(source, Path({source}, __MIN_COST__)));

  while (!queue.empty() && k_paths.size() != k) {
    auto [vertex, path] = queue.top();

    queue.pop();

    if (vertex == destination) {
      k_paths.push_back(path);

      continue;
    }

    for (const auto &edge : adjacency_list[vertex]) {
      auto new_path{path};

      new_path.vertices.push_back(edge.destination);

      queue.push(Binding(edge.destination,
                         Path(new_path.vertices, path.cost + edge.cost)));
    }
  }

  return k_paths;
}

auto Graph::random_path(void) noexcept -> Path {
  std::random_device random_device;

  Uniform distribution{random_device(), 0, static_cast<double>(size())};

  auto source{static_cast<std::size_t>(distribution.next())};

  auto destination{static_cast<std::size_t>(distribution.next())};

  while (true) {
    destination = static_cast<std::size_t>(distribution.next());

    const auto path{dijkstra(source, destination)};

    if (!path.vertices.empty()) {
      return path;
    }
  }
}

auto Graph::random_source_destination(void) noexcept
    -> std::pair<Vertex, Vertex> {
  const auto path{random_path()};

  return std::make_pair(path.source(), path.destination());
}

auto Graph::paths(void) noexcept -> std::vector<Path> {
  std::vector<Path> paths{};

  for (auto source{0u}; source < size(); ++source) {
    for (const auto &[destination, cost] : adjacency_list.at(source)) {
      const auto path{dijkstra(source, destination)};

      if (path.vertices.empty()) {
        continue;
      }

      paths.push_back(path);
    }
  }

  return paths;
}

auto Graph::at(const Vertex source, const Vertex destination) const -> Cost {
  for (const auto &[vertex, cost] : adjacency_list.at(source)) {
    if (vertex == destination) {
      return cost;
    }
  }

  return __MIN_COST__;
}

auto Graph::adjacent(const Vertex source, const Vertex destination) const
    -> bool {
  return at(source, destination) != __MIN_COST__;
}

auto Graph::add_vertex(const Vertex vertex) -> void {
  vertices[vertex] = vertex;
}

auto Graph::add_edge(const Vertex source, const Vertex destination,
                     const Cost cost) -> void {
  adjacency_list[source].emplace_back(destination, cost);
}

auto operator<<(std::ostream &stream, const Graph &graph) -> std::ostream & {
  return stream << graph.to_string();
}
