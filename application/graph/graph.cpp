#include "graph.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <format>
#include <fstream>
#include <ranges>
#include <sstream>

namespace graph {
struct Graph::Implementation {
  typedef boost::property<boost::edge_weight_t, Cost> EdgeWeightProperty;
  typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                                boost::no_property, EdgeWeightProperty>
      BoostGraph;

  BoostGraph graph;

  Implementation(void) = default;

  Implementation(const uint64_t vertices) : graph(vertices) {}

  Implementation(const BoostGraph& other_graph) : graph(other_graph) {}

  uint64_t size() const { return boost::num_vertices(graph); }

  Cost at(const Vertex source, const Vertex destination) const {
    auto [edge, exists] = boost::edge(source, destination, graph);

    return exists ? boost::get(boost::edge_weight, graph, edge) : Cost::min();
  }

  std::list<AdjacentVertex> at(const Vertex source) const {
    std::list<AdjacentVertex> adjacents;

    auto [begin, end] = boost::adjacent_vertices(source, graph);

    for (auto iterator = begin; iterator != end; ++iterator) {
      auto [edge, exists] = boost::edge(source, *iterator, graph);

      if (!exists) {
        continue;
      }

      Cost cost = boost::get(boost::edge_weight, graph, edge);

      adjacents.emplace_back(*iterator, cost);
    }

    return adjacents;
  }

  bool is_adjacent(const Vertex source, const Vertex destination) const {
    auto [_, exists] = boost::edge(source, destination, graph);

    return exists;
  }

  std::set<Vertex> get_vertices() const {
    std::set<Vertex> vertices;

    auto [begin, end] = boost::vertices(graph);

    for (auto iterator = begin; iterator != end; ++iterator) {
      vertices.insert(static_cast<Vertex>(*iterator));
    }

    return vertices;
  }

  std::vector<Edge> get_edges() const {
    std::vector<Edge> edges;

    auto [begin, end] = boost::edges(graph);

    for (auto iterator = begin; iterator != end; ++iterator) {
      edges.push_back({static_cast<Vertex>(boost::source(*iterator, graph)),
                       static_cast<Vertex>(boost::target(*iterator, graph)),
                       boost::get(boost::edge_weight, graph, *iterator)});
    }

    return edges;
  }

  void add(const Vertex vertex) {
    while (vertex >= boost::num_vertices(graph)) {
      boost::add_vertex(graph);
    }
  }

  void add(const Edge& edge) {
    const auto& [source, destination, cost] = edge;

    boost::add_edge(source, destination, EdgeWeightProperty(cost), graph);
  }
};

Graph::Graph(void) : pImpl(std::make_unique<Implementation>()) {}

Graph::Graph(const uint64_t vertices)
    : pImpl(std::make_unique<Implementation>(vertices)) {}

Graph::~Graph() = default;

Graph::Graph(const Graph& other)
    : pImpl(std::make_unique<Implementation>(other.pImpl->graph)) {}

Graph& Graph::operator=(const Graph& other) {
  if (this != &other) {
    pImpl = std::make_unique<Implementation>(other.pImpl->graph);
  }
  return *this;
}

Graph::Graph(Graph&&) noexcept = default;
Graph& Graph::operator=(Graph&&) noexcept = default;

std::optional<Graph> Graph::from(const std::string& filename) noexcept {
  std::ifstream file{filename};

  if (!file.is_open()) {
    return std::nullopt;
  }

  std::string line{};

  std::getline(file, line);

  const auto size{static_cast<uint64_t>(atoi(line.c_str()))};

  Graph graph{size};

  auto source{0u};

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    for (const auto& destination : std::views::iota(0u, size)) {
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

uint64_t Graph::size(void) const noexcept { return pImpl->size(); }

Cost Graph::at(const Vertex source, const Vertex destination) const {
  return pImpl->at(source, destination);
}

std::list<AdjacentVertex> Graph::at(const Vertex vertex) const {
  return pImpl->at(vertex);
}

bool Graph::is_adjacent(const Vertex source, const Vertex destination) const {
  return pImpl->is_adjacent(source, destination);
}

std::set<Vertex> Graph::get_vertices(void) const noexcept {
  return pImpl->get_vertices();
}

std::vector<Edge> Graph::get_edges(void) const noexcept {
  return pImpl->get_edges();
}

void Graph::add(const Vertex vertex) { pImpl->add(vertex); }

void Graph::add(const Edge& edge) { pImpl->add(edge); }
}  // namespace graph
