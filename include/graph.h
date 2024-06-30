#pragma once

#include <limits>
#include <list>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

using Vertex = std::size_t;

using Path = std::unordered_set<Vertex>;

using Weight = float;

constexpr auto __MAX_WEIGHT__ = std::numeric_limits<Weight>::max();

constexpr auto __MIN_WEIGHT__ = static_cast<Weight>(0);

using Edge = std::tuple<Vertex, Vertex, Weight>;

class Graph final {
public:
  Graph(const std::size_t);

  [[nodiscard]] static auto from(const std::string &) noexcept
      -> std::optional<Graph>;

  [[nodiscard]] auto size(void) const noexcept -> std::size_t;

  [[nodiscard]] auto contains(const Vertex) const noexcept -> bool;

  [[nodiscard]] auto to_string(void) const noexcept -> std::string;

  auto add_edge(const Edge &) -> void;

  auto add_edge(const Vertex, const Vertex, const Weight) -> void;

private:
  std::vector<std::list<std::pair<Vertex, Weight>>> adjacency_list;
};

[[nodiscard]] auto operator<<(std::ostream &, const Graph &) -> std::ostream &;
