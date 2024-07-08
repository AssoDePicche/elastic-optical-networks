#include "graph.h"

#include <algorithm>
#include <fstream>
#include <memory>
#include <queue>
#include <sstream>
#include <utility>

#include "distribution.h"

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

  auto origin{0u};

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    for (auto destination{0u}; destination < size; ++destination) {
      std::getline(stream, buffer, ' ');

      const auto weight = static_cast<Weight>(atof(buffer.c_str()));

      if (weight != __MIN_WEIGHT__) {
        graph.add_edge(origin, destination, weight);
      }
    }

    ++origin;
  }

  return graph;
}

auto Graph::size(void) const noexcept -> std::size_t { return vertices.size(); }

auto Graph::to_string(void) const noexcept -> std::string {
  std::string buffer{};

  auto matrix = std::vector(size(), std::vector(size(), __MIN_WEIGHT__));

  for (const auto &[vertex, edges] : adjacency_list) {
    for (auto &[adjacent, weight, spectrum] : edges) {
      matrix[vertex][adjacent] = weight;
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

auto Graph::dijkstra(const Vertex origin, const Vertex destination) noexcept
    -> Path {
  if (origin == destination) {
    return {};
  }

  std::unordered_map<int, Weight> weights;

  std::unordered_map<int, int> predecessors;

  std::unordered_map<int, int> edge_hops;

  // {weight, {hops, vertex}}
  using PathInfo = std::pair<Weight, std::pair<int, int>>;

  std::priority_queue<PathInfo, std::vector<PathInfo>, std::greater<>> queue;

  for (const auto &[id, name] : vertices) {
    weights[id] = __MAX_WEIGHT__;

    predecessors[id] = -1;

    edge_hops[id] = __MAX_HOPS__;
  }

  weights[origin] = __MIN_WEIGHT__;

  edge_hops[origin] = __MIN_HOPS__;

  queue.emplace(weights[origin], std::make_pair(edge_hops[origin], origin));

  while (!queue.empty()) {
    const auto current_weight = queue.top().first;

    const auto hops = queue.top().second.first;

    const auto vertex = queue.top().second.second;

    queue.pop();

    if (vertex == static_cast<int>(destination)) {
      break;
    }

    if (current_weight > weights[vertex]) {
      continue;
    }

    if (current_weight == weights[vertex] && hops > edge_hops[vertex]) {
      continue;
    }

    for (const auto &[adjacent, weight, spectrum] : adjacency_list[vertex]) {
      const auto new_weight = current_weight + weight;

      if (new_weight > weights[adjacent]) {
        continue;
      }

      const auto new_hops = hops + 1;

      const auto less_hops = (new_hops < edge_hops[adjacent]);

      if (new_weight == weights[adjacent] && !less_hops) {
        continue;
      }

      weights[adjacent] = new_weight;

      edge_hops[adjacent] = new_hops;

      predecessors[adjacent] = vertex;

      queue.emplace(new_weight, std::make_pair(new_hops, adjacent));
    }
  }

  Path path;

  auto current = static_cast<int>(destination);

  while (current != -1) {
    path.push_back(static_cast<std::size_t>(current));

    current = predecessors[current];
  }

  std::reverse(path.begin(), path.end());

  if (path.front() != origin) {
    path.clear();
  }

  return path;
}

auto Graph::random_path(void) noexcept -> Path {
  auto distribution = std::make_unique<Uniform>(0, size() - 1);

  auto origin = static_cast<std::size_t>(distribution->next());

  auto destination = static_cast<std::size_t>(distribution->next());

  Path path;

  while (path.empty()) {
    destination = static_cast<std::size_t>(distribution->next());

    path = dijkstra(origin, destination);
  }

  return path;
}

auto Graph::add_vertex(const Vertex vertex) -> void {
  vertices[vertex] = vertex;
}

auto Graph::add_edge(const Vertex origin, const Vertex destination,
                     const Weight weight) -> void {
  adjacency_list[origin].emplace_back(destination, weight);
}

auto operator<<(std::ostream &stream, const Graph &graph) -> std::ostream & {
  return stream << graph.to_string();
}
