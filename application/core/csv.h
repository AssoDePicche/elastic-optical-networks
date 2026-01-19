#pragma once

#include <string>

namespace core {
[[nodiscard]] std::string csv_to_graphviz(const std::string&);

[[nodiscard]] std::string csv_to_txt(const std::string&);
}  // namespace core
