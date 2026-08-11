#pragma once

#include <memory>
#include <unordered_set>

#include "graph.h"
#include "path.h"

namespace core {
class Dijkstra {
 public:
  using Edge = std::pair<Graph::Vertex, Graph::Vertex>;

  Dijkstra(const Graph&);

  ~Dijkstra();

  [[nodiscard]] Path compute(const Graph::Vertex, const Graph::Vertex) const;

  [[nodiscard]] Path compute(const Graph::Vertex, const Graph::Vertex,
                             const std::set<Edge>&,
                             const std::unordered_set<Graph::Vertex>&) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
