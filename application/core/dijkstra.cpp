#include "dijkstra.h"

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <vector>

namespace core {
using EdgeWeightProperty = boost::property<boost::edge_weight_t, double>;
using BoostGraph =
    boost::adjacency_list<boost::listS, boost::vecS, boost::directedS,
                          boost::no_property, EdgeWeightProperty>;
using VertexDescriptor = boost::graph_traits<BoostGraph>::vertex_descriptor;

Path reconstruct_path(const std::vector<VertexDescriptor>& predecessors,
                      const std::vector<double>& distances, Vertex source,
                      Vertex destination) {
  const auto num_nodes = distances.size();
  if (source >= num_nodes || destination >= num_nodes) {
    return {};
  }
  if (distances[destination] >= std::numeric_limits<double>::max() / 2.0) {
    return {};
  }

  std::vector<Vertex> reverse_path;
  for (Vertex v = destination; v != source; v = predecessors[v]) {
    reverse_path.push_back(v);
    if (predecessors[v] == v) return {};
  }
  reverse_path.push_back(source);

  Path result;
  result.vertices.assign(reverse_path.rbegin(), reverse_path.rend());
  result.cost = distances[destination];
  return result;
}

BoostGraph build_bgl_graph(
    const Graph& g, const std::set<std::pair<Vertex, Vertex>>& disabled_edges,
    const std::unordered_set<Vertex>& disabled_vertices) {
  BoostGraph bg(g.size());
  for (const auto& [source, destination, cost] : g.get_edges()) {
    if (disabled_vertices.contains(source) ||
        disabled_vertices.contains(destination)) {
      continue;
    }
    if (disabled_edges.contains({source, destination})) {
      continue;
    }
    boost::add_edge(source, destination, EdgeWeightProperty(cost.value), bg);
  }
  return bg;
}

struct Dijkstra::Implementation {
  const Graph graph;

  Implementation(const Graph& graph) : graph{graph} {}

  Path compute(const Vertex source, const Vertex destination) const {
    return compute(source, destination, {}, {});
  }

  Path compute(const Vertex source, const Vertex destination,
               const std::set<std::pair<Vertex, Vertex>>& disabled_edges,
               const std::unordered_set<Vertex>& disabled_vertices) const {
    BoostGraph bg = build_bgl_graph(graph, disabled_edges, disabled_vertices);

    std::vector<VertexDescriptor> predecessors(boost::num_vertices(bg));
    std::vector<double> distances(boost::num_vertices(bg),
                                  std::numeric_limits<double>::max());

    boost::dijkstra_shortest_paths(
        bg, source,
        boost::predecessor_map(
            boost::make_iterator_property_map(
                predecessors.begin(), boost::get(boost::vertex_index, bg)))
            .distance_map(boost::make_iterator_property_map(
                distances.begin(), boost::get(boost::vertex_index, bg))));

    return reconstruct_path(predecessors, distances, source, destination);
  }
};

Dijkstra::Dijkstra(const Graph& graph)
    : pImpl(std::make_unique<Implementation>(graph)) {}

Dijkstra::~Dijkstra() = default;

Path Dijkstra::compute(const Vertex source, const Vertex destination) const {
  return pImpl->compute(source, destination);
}

Path Dijkstra::compute(
    const Vertex source, const Vertex destination,
    const std::set<Edge>& disabled_edges,
    const std::unordered_set<Vertex>& disabled_vertices) const {
  return pImpl->compute(source, destination, disabled_edges, disabled_vertices);
}
}  // namespace core
