#pragma once

#include <limits>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

using Vertex = unsigned;

constexpr Vertex NullVertex = std::numeric_limits<unsigned>::max();

struct Cost {
  double value;

  Cost(void) = default;

  Cost(double);

  static Cost max(void);

  static Cost min(void);
};

[[nodiscard]] bool operator<(const Cost &, const Cost &);

using AdjacentVertex = std::pair<Vertex, Cost>;

using Edge = std::tuple<Vertex, Vertex, Cost>;

class Graph final {
 public:
  Graph(void) = default;

  Graph(const unsigned);

  [[nodiscard]] static std::optional<Graph> from(const std::string &) noexcept;

  [[nodiscard]] unsigned size(void) const noexcept;

  [[nodiscard]] Cost at(const Vertex, const Vertex) const;

  [[nodiscard]] std::list<AdjacentVertex> at(const Vertex) const;

  [[nodiscard]] bool is_adjacent(const Vertex, const Vertex) const;

  [[nodiscard]] std::set<Vertex> get_vertices(void) const noexcept;

  [[nodiscard]] std::vector<Edge> get_edges(void) const noexcept;

  void add(const Vertex);

  void add(const Edge &);

 private:
  std::unordered_map<Vertex, std::list<AdjacentVertex>> adjacency_list;
  std::set<Vertex> vertices;
};
