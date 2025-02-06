#pragma once

#include <limits>
#include <list>
#include <optional>
#include <ostream>
#include <set>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

using vertex_t = unsigned;

using cost_t = double;

struct Cost {
  static constexpr auto max = std::numeric_limits<cost_t>::max();
  static constexpr auto min = static_cast<cost_t>(0);
};

constexpr auto __MAX_HOPS__ = std::numeric_limits<unsigned>::max();

constexpr auto __MIN_HOPS__ = static_cast<unsigned>(0);

using adjacent_t = std::pair<vertex_t, cost_t>;

struct edge_t final {
  cost_t cost;
  vertex_t source;
  vertex_t destination;
};

int operator<=>(const edge_t &, const edge_t &);

using route_t = std::pair<std::unordered_set<vertex_t>, cost_t>;

const route_t __NO_ROUTE__ = {{}, Cost::max};

class Graph final {
 public:
  Graph(void) = default;

  Graph(const unsigned);

  [[nodiscard]] static auto from(const std::string &) noexcept
      -> std::optional<Graph>;

  [[nodiscard]] auto size(void) const noexcept -> unsigned;

  [[nodiscard]] auto to_string(void) const noexcept -> std::string;

  [[nodiscard]] auto at(const vertex_t, const vertex_t) const -> cost_t;

  [[nodiscard]] auto at(const vertex_t) const -> std::list<adjacent_t>;

  [[nodiscard]] auto is_adjacent(const vertex_t, const vertex_t) const -> bool;

  [[nodiscard]] auto get_vertices(void) const noexcept -> std::set<vertex_t>;

  [[nodiscard]] auto get_edges(void) const noexcept -> std::set<edge_t>;

  auto add(const vertex_t) -> void;

  auto add(const vertex_t, const vertex_t, const cost_t) -> void;

 private:
  std::unordered_map<vertex_t, std::list<adjacent_t>> adjacency_list;
  std::set<vertex_t> vertices;
};

[[nodiscard]] auto operator<<(std::ostream &, const Graph &) -> std::ostream &;

[[nodiscard]] auto breadth_first_search(const Graph &, const vertex_t,
                                        const vertex_t) noexcept -> route_t;

[[nodiscard]] auto depth_first_search(const Graph &, const vertex_t,
                                      const vertex_t) noexcept -> route_t;

[[nodiscard]] auto dijkstra(const Graph &, const vertex_t,
                            const vertex_t) noexcept -> route_t;

[[nodiscard]] auto k_shortest_path(const Graph &, const vertex_t,
                                   const vertex_t, const unsigned) noexcept
    -> std::vector<route_t>;

[[nodiscard]] auto random_path(const Graph &) noexcept -> route_t;
