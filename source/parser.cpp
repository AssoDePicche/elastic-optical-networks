#include "parser.h"

#include <algorithm>

Parser::Parser(const int argc, const char **argv) {
  for (auto index{1}; index < argc; ++index) {
    buffer.push_back(std::string(argv[index]));
  }
}

auto Parser::contains(const std::string &arg) const -> bool {
  return std::find(buffer.begin(), buffer.end(), arg) != buffer.end();
}

auto Parser::parse(const std::string &arg) const -> std::string {
  auto ptr = std::find(buffer.begin(), buffer.end(), arg);

  if (ptr != buffer.end() && ++ptr != buffer.end()) {
    return *ptr;
  }

  return std::string("");
}
