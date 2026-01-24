#include "csv.h"

#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

namespace graph {
std::string csv_to_graphviz(const std::string& filename) {
  std::ifstream csv(filename);

  if (!csv.is_open()) {
    return "";
  }

  std::string graphviz;

  graphviz.append("graph G {\n");

  auto next = [](std::stringstream& stream, std::string& buffer) {
    std::getline(stream, buffer, ',');

    return buffer;
  };

  std::string line{};

  std::getline(csv, line);  // skip table header

  while (std::getline(csv, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    const auto source{next(stream, buffer)};

    const auto destination{next(stream, buffer)};

    const auto weight{next(stream, buffer)};

    graphviz.append(std::format("  \"{}\" -- \"{}\" [weight={},label={}]\n",
                                source, destination, weight, weight));
  }

  graphviz.append("}\n");

  return graphviz;
}

std::set<std::string> nodes_from(const std::string& filename) {
  std::set<std::string> nodes{};

  std::ifstream file{filename};

  std::string line{};

  std::getline(file, line);  // skip header

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string node{};

    std::getline(stream, node, ',');

    nodes.insert(node);

    std::getline(stream, node, ',');

    nodes.insert(node);
  }

  file.close();

  return nodes;
}

std::vector<std::vector<double>> matrix_from(const std::string& filename) {
  std::ifstream file{filename};

  if (!file.is_open()) {
    return {};
  }

  const auto nodes{nodes_from(filename)};

  auto matrix{std::vector(nodes.size(), std::vector(nodes.size(), 0.0))};

  std::string line{};

  std::getline(file, line);  // skip header

  while (std::getline(file, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    std::getline(stream, buffer, ',');

    const auto source{buffer};

    std::getline(stream, buffer, ',');

    const auto destination{buffer};

    std::getline(stream, buffer, ',');

    const auto distance{std::atof(buffer.c_str())};

    const auto i = std::distance(nodes.begin(), nodes.find(source));

    const auto j = std::distance(nodes.begin(), nodes.find(destination));

    matrix[i][j] = distance;

    matrix[j][i] = distance;
  }

  file.close();

  std::cout << nodes.size() << std::endl;

  return matrix;
}

std::string csv_to_txt(const std::string& filename) {
  const auto matrix{matrix_from(filename)};

  std::string buffer;

  for (const auto& row : matrix) {
    for (const auto& column : row) {
      buffer.append(std::format("{} ", column));
    }

    buffer.append("\n");
  }

  return buffer;
}
}  // namespace graph
