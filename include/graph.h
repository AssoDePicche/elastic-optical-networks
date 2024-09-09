#pragma once

#include <limits>
#include <list>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

using Vertex = std::size_t;

using Cost = double;

constexpr auto __MAX_COST__ = std::numeric_limits<Cost>::max();

constexpr auto __MIN_COST__ = static_cast<Cost>(0);

struct Path {
  std::vector<Vertex> vertices{};
  Cost cost{};

  Path(void) = default;

  Path(const std::vector<Vertex> &, const Cost);

  [[nodiscard]] auto source(void) const -> Vertex;

  [[nodiscard]] auto destination(void) const -> Vertex;

  [[nodiscard]] auto operator>(const Path &) const -> bool;
};

constexpr auto __MAX_HOPS__ = std::numeric_limits<int>::max();

constexpr auto __MIN_HOPS__ = static_cast<int>(0);

struct Edge final {
  Vertex destination;
  Cost cost;
};

class Graph final {
public:
  Graph(void) = default;

  Graph(const std::size_t);

  [[nodiscard]] static auto from(const std::string &) noexcept
      -> std::optional<Graph>;

  [[nodiscard]] auto size(void) const noexcept -> std::size_t;

  [[nodiscard]] auto to_string(void) const noexcept -> std::string;

  [[nodiscard]] auto breadth_first_search(const Vertex, const Vertex) noexcept
      -> Path;

  [[nodiscard]] auto depth_first_search(const Vertex, const Vertex) noexcept
      -> Path;

  [[nodiscard]] auto dijkstra(const Vertex, const Vertex) noexcept -> Path;

  [[nodiscard]] auto k_shortest_path(const Vertex, const Vertex,
                                     const unsigned) noexcept
      -> std::vector<Path>;

  [[nodiscard]] auto random_path(void) noexcept -> Path;

  [[nodiscard]] auto random_source_destination(void) noexcept
      -> std::pair<Vertex, Vertex>;

  [[nodiscard]] auto paths(void) noexcept -> std::vector<Path>;

  [[nodiscard]] auto at(const Vertex, const Vertex) const -> Cost;

  [[nodiscard]] auto adjacent(const Vertex, const Vertex) const -> bool;

  auto add_vertex(const Vertex) -> void;

  auto add_edge(const Vertex, const Vertex, const Cost) -> void;

private:
  std::unordered_map<std::size_t, Vertex> vertices;
  std::unordered_map<std::size_t, std::list<Edge>> adjacency_list;
};

[[nodiscard]] auto operator<<(std::ostream &, const Graph &) -> std::ostream &;
