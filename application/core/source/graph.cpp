#include "graph.h"

#include <format>
#include <fstream>
#include <limits>
#include <ranges>
#include <sstream>

Cost::Cost(double value) : value{value} {}

Cost Cost::max(void) { return Cost(std::numeric_limits<double>::max()); }

Cost Cost::min(void) { return Cost(.0f); }

bool operator<(const Cost &lhs, const Cost &rhs) {
  return lhs.value < rhs.value;
}

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
