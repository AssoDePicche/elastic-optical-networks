#pragma once

#include <string>

namespace graph {
[[nodiscard]] std::string csv_to_graphviz(const std::string&);

[[nodiscard]] std::string csv_to_txt(const std::string&);
}  // namespace graph
