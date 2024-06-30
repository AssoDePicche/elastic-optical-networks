#include "graph.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

Graph::Graph(const std::size_t size) {
  adjacency_list = std::vector(size, std::list<std::pair<Vertex, Weight>>{});
}

auto Graph::size(void) const noexcept -> std::size_t {
  return adjacency_list.size();
}

auto Graph::contains(const Vertex vertex) const noexcept -> bool {
  return vertex < size();
}

auto Graph::to_string(void) const noexcept -> std::string {
  std::string buffer{};

  auto adjacency_matrix =
      std::vector(size(), std::vector(size(), __MIN_WEIGHT__));

  for (auto origin{0u}; origin < size(); ++origin) {
    const auto row{adjacency_list[origin]};

    for (auto iterator{row.begin()}; iterator != row.end(); ++iterator) {
      const auto [destination, weight] = *iterator;

      adjacency_matrix[origin][destination] = weight;
    }
  }

  for (const auto &row : adjacency_matrix) {
    for (const auto &column : row) {
      buffer.append(std::to_string(column) + ' ');
    }

    buffer.append("\n");
  }

  return buffer;
}

auto Graph::add_edge(const Edge &edge) -> void {
  const auto [origin, destination, weight] = edge;

  add_edge(origin, destination, weight);
}

auto Graph::add_edge(const Vertex origin, const Vertex destination,
                     const Weight weight) -> void {
  if (!contains(origin) || !contains(destination)) {
    throw std::invalid_argument("Vertices must belong to the graph.");
  }

  if (weight < __MIN_WEIGHT__) {
    throw std::invalid_argument("The weight must be above zero.");
  }

  adjacency_list[origin].push_back({destination, weight});
}

auto Graph::from(const std::string &filename) noexcept -> std::optional<Graph> {
  std::ifstream file{filename};

  if (!file.is_open()) {
    return std::nullopt;
  }

  std::string line{};

  std::getline(file, line);

  const auto size{static_cast<std::size_t>(atoi(line.c_str()))};

  Graph graph{size};

  auto origin{0u};

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    for (auto destination{0u}; destination < size; ++destination) {
      std::getline(stream, buffer, ' ');

      const auto weight = static_cast<Weight>(atof(buffer.c_str()));

      if (weight != __MIN_WEIGHT__) {
        graph.add_edge({origin, destination, weight});
      }
    }

    ++origin;
  }

  return graph;
}

auto operator<<(std::ostream &stream, const Graph &graph) -> std::ostream & {
  return stream << graph.to_string();
}
