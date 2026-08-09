#pragma once

#include <string>

namespace core {
class FileSystem final {
 public:
  [[nodiscard]] static bool CreateDirectory(const std::string&);

  [[nodiscard]] static bool Exists(const std::string&);
};
}  // namespace core
