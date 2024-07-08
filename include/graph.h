#pragma once

#include <limits>
#include <list>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <vector>

using Vertex = std::size_t;

using Path = std::vector<Vertex>;

constexpr auto __MAX_HOPS__ = std::numeric_limits<int>::max();

constexpr auto __MIN_HOPS__ = static_cast<int>(0);

using Weight = double;

constexpr auto __MAX_WEIGHT__ = std::numeric_limits<Weight>::max();

constexpr auto __MIN_WEIGHT__ = static_cast<Weight>(0);

struct Edge final {
  Vertex destination;
  Weight weight;
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

  [[nodiscard]] auto random_path(void) noexcept -> Path;

  auto add_vertex(const Vertex) -> void;

  auto add_edge(const Vertex, const Vertex, const Weight) -> void;

private:
  std::unordered_map<std::size_t, Vertex> vertices;
  std::unordered_map<std::size_t, std::list<Edge>> adjacency_list;
};

[[nodiscard]] auto operator<<(std::ostream &, const Graph &) -> std::ostream &;
