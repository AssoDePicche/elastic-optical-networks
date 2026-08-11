#pragma once

#include <string>

namespace core::file_system {
[[nodiscard]] bool create_directory(const std::string&);

[[nodiscard]] bool exists(const std::string&);
}  // namespace core::file_system
