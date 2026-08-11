#include "yen.h"

#include <queue>
#include <set>
#include <string>
#include <unordered_set>

#include "dijkstra.h"

namespace core {
bool shares_prefix(const Path& self, const Path& other, size_t index) {
  if (self.vertices.size() <= index || other.vertices.size() <= index) {
    return false;
  }
  for (size_t i = 0; i <= index; ++i) {
    if (self.vertices[i] != other.vertices[i]) return false;
  }
  return true;
}

struct Yen::Implementation {
  Graph graph;

  Implementation(const Graph& graph) : graph(graph) {}

  std::vector<Path> compute(const Graph::Vertex source,
                            const Graph::Vertex destination,
                            const uint8_t k) const {
    std::vector<Path> A;
    if (k == 0) return A;

    Dijkstra dijkstra(graph);

    Path first_path = dijkstra.compute(source, destination);
    if (first_path.empty()) return A;
    A.push_back(first_path);

    auto comp = [](const Path& p1, const Path& p2) {
      return p1.cost > p2.cost;
    };
    std::priority_queue<Path, std::vector<Path>, decltype(comp)> B(comp);

    auto path_key = [](const Path& p) {
      std::string s;
      for (auto v : p.vertices) s += std::to_string(v) + "-";
      return s;
    };
    std::unordered_set<std::string> candidate_keys;

    for (size_t k_idx = 1; k_idx < k; ++k_idx) {
      const Path& last_path = A.back();

      for (size_t i = 0; i < last_path.vertices.size() - 1; ++i) {
        Graph::Vertex spur_node = last_path.vertices[i];

        std::vector<Graph::Vertex> root_path_vec(
            last_path.vertices.begin(), last_path.vertices.begin() + i + 1);

        std::set<std::pair<Graph::Vertex, Graph::Vertex>> disabled_edges;
        std::unordered_set<Graph::Vertex> disabled_vertices;

        for (const auto& path : A) {
          if (path.vertices.size() > i && shares_prefix(path, last_path, i)) {
            disabled_edges.insert({path.vertices[i], path.vertices[i + 1]});
          }
        }

        for (size_t r = 0; r < i; ++r) {
          disabled_vertices.insert(root_path_vec[r]);
        }

        Path spur_path = dijkstra.compute(spur_node, destination,
                                          disabled_edges, disabled_vertices);

        if (!spur_path.empty()) {
          Path total_candidate;
          total_candidate.vertices = root_path_vec;
          total_candidate.vertices.insert(total_candidate.vertices.end(),
                                          spur_path.vertices.begin() + 1,
                                          spur_path.vertices.end());

          double total_cost = 0.0;
          for (size_t e = 0; e < total_candidate.vertices.size() - 1; ++e) {
            total_cost += graph.at(total_candidate.vertices[e],
                                   total_candidate.vertices[e + 1]);
          }
          total_candidate.cost = total_cost;

          std::string key = path_key(total_candidate);
          if (!candidate_keys.contains(key)) {
            B.push(total_candidate);
            candidate_keys.insert(key);
          }
        }
      }

      if (B.empty()) break;

      A.push_back(B.top());
      B.pop();
    }

    return A;
  }
};

Yen::Yen(const Graph& graph) : pImpl(std::make_unique<Implementation>(graph)) {}

Yen::~Yen() = default;

std::vector<Path> Yen::compute(const Graph::Vertex source,
                               const Graph::Vertex destination,
                               const uint8_t k) const {
  return pImpl->compute(source, destination, k);
}
};  // namespace core
