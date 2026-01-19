#pragma once

#include <format>
#include <fstream>
#include <string>

namespace core {
class Document final {
 public:
  template <typename... Args>
  Document& append(std::format_string<Args...> format, Args&&... args) {
    buffer.append(std::format(format, std::forward<Args>(args)...));

    return *this;
  }

  [[nodiscard]] inline std::string Build(void) const { return buffer; }

  void write(const std::string& filename) {
    std::ofstream stream(filename);

    if (!stream.is_open()) {
      throw std::runtime_error(std::format("Failed to open {}", filename));
    }

    stream << buffer;
  }

 private:
  std::string buffer;
};
}  // namespace core
