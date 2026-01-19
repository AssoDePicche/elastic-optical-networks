#pragma once

namespace core {
class FileSystem final {
 public:
  [[nodiscard]] static bool CreateDirectory(const std::string&);
};
}  // namespace core
