#pragma once

#include <cstdint>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>

#include "cost.h"
#include "edge.h"
#include "vertex.h"

namespace graph {
class Graph final {
  std::unordered_map<Vertex, std::list<AdjacentVertex>> adjacency_list;
  std::set<Vertex> vertices;

 public:
  Graph(void) = default;

  Graph(const uint64_t);

  [[nodiscard]] static std::optional<Graph> from(const std::string&) noexcept;

  [[nodiscard]] uint64_t size(void) const noexcept;

  [[nodiscard]] Cost at(const Vertex, const Vertex) const;

  [[nodiscard]] std::list<AdjacentVertex> at(const Vertex) const;

  [[nodiscard]] bool is_adjacent(const Vertex, const Vertex) const;

  [[nodiscard]] std::set<Vertex> get_vertices(void) const noexcept;

  [[nodiscard]] std::vector<Edge> get_edges(void) const noexcept;

  void add(const Vertex);

  void add(const Edge&);
};
}  // namespace graph
