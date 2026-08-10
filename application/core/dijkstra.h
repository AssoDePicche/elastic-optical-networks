#pragma once

#include <memory>
#include <unordered_set>

#include "graph.h"
#include "path.h"

namespace core {
class Dijkstra {
 public:
  using Edge = std::pair<Vertex, Vertex>;

  Dijkstra(const Graph&);

  ~Dijkstra();

  [[nodiscard]] Path compute(const Vertex, const Vertex) const;

  [[nodiscard]] Path compute(const Vertex, const Vertex, const std::set<Edge>&,
                             const std::unordered_set<Vertex>&) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
