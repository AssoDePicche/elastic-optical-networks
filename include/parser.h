#pragma once

#include <string>
#include <vector>

class Parser final {
public:
  Parser(const std::vector<std::string> &);

  [[nodiscard]] auto contains(const std::string &) const -> bool;

  [[nodiscard]] auto parse(const std::string &) const -> std::string;

private:
  std::vector<std::string> buffer;
};
