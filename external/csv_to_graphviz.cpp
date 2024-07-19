#include <fstream>
#include <iostream>
#include <sstream>

auto format(const std::string &u, const std::string &v, const std::string &w)
    -> std::string {
  auto q = [](const std::string &str) { return '"' + str + '"'; };

  return "  " + q(u) + " -- " + q(v) + " [weight=" + w + ",label=" + w + "]\n";
}

auto main(const int argc, const char **argv) -> int {
  if (argc != 2) {
    std::cerr << "Pass the .csv filename as an argument." << std::endl;

    return 1;
  }

  std::string filename{argv[1]};

  std::ifstream csv{filename + ".csv"};

  if (!csv.is_open()) {
    std::cerr << "Could not read the " << filename << ".csv file." << std::endl;

    return 1;
  }

  std::ofstream graphviz{filename + ".gv"};

  if (!graphviz.is_open()) {
    std::cerr << "Could not write the " << filename << ".gv file." << std::endl;

    return 1;
  }

  graphviz << "graph G {\n";

  auto next = [](std::stringstream &stream, std::string &buffer) {
    std::getline(stream, buffer, ',');

    return buffer;
  };

  std::string line{};

  std::getline(csv, line); // skip table header

  while (std::getline(csv, line)) {
    std::stringstream stream{line};

    std::string buffer{};

    const auto source{next(stream, buffer)};

    const auto destination{next(stream, buffer)};

    const auto weight{next(stream, buffer)};

    graphviz << format(source, destination, weight);
  }

  graphviz << "}\n";
}
