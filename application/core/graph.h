#pragma once

#include <cstdint>
#include <limits>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

namespace core {
class Graph final {
 public:
  using Vertex = uint8_t;

  struct Edge {
    using Cost = double;
    static constexpr Cost MAX_COST = std::numeric_limits<Cost>::max();
    static constexpr Cost MIN_COST = 0;
    Vertex source;
    Vertex destination;
    Cost cost;
  };

  Graph(void);

  Graph(const uint8_t);

  ~Graph();

  Graph(const Graph& other);

  Graph& operator=(const Graph& other);

  Graph(Graph&&) noexcept;

  Graph& operator=(Graph&&) noexcept;

  [[nodiscard]] static std::optional<Graph> from(const std::string&) noexcept;

  [[nodiscard]] uint8_t size(void) const noexcept;

  [[nodiscard]] Edge::Cost at(const Vertex, const Vertex) const;

  [[nodiscard]] bool is_adjacent(const Vertex, const Vertex) const;

  [[nodiscard]] std::set<Vertex> get_vertices(void) const noexcept;

  [[nodiscard]] std::vector<Edge> get_edges(void) const noexcept;

  void add(const Vertex);

  void add(const Edge&);

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
