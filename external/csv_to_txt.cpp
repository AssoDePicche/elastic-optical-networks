#include <fstream>
#include <iostream>
#include <iterator>
#include <set>
#include <sstream>
#include <vector>

auto nodes_from(const std::string &filename) -> std::set<std::string> {
  std::set<std::string> nodes{};

  std::ifstream file{filename};

  std::string line{};

  std::getline(file, line); // skip header

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

auto matrix_from(const std::string &filename)
    -> std::vector<std::vector<double>> {
  std::ifstream file{filename};

  if (!file.is_open()) {
    std::cerr << "Could not read the file." << std::endl;
  }

  const auto nodes{nodes_from(filename)};

  auto matrix{std::vector(nodes.size(), std::vector(nodes.size(), 0.0))};

  std::string line{};

  std::getline(file, line); // skip header

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

auto main(const int argc, const char **argv) -> int {
  if (argc < 2) {
    std::cerr << "Pass the .csv filename as an argument." << std::endl;

    return 1;
  }

  const auto filename{std::string(argv[1])};

  const auto matrix{matrix_from(filename)};

  for (const auto &row : matrix) {
    for (const auto &column : row) {
      std::cout << column << ' ';
    }

    std::cout << std::endl;
  }
}
