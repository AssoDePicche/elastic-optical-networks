#pragma once

#include <functional>
#include <string>

namespace core {
class Application final {
 public:
  [[nodiscard]] int Run(const int, const char**);

 private:
  [[nodiscard]] double Benchmark(std::function<void()>);

  [[nodiscard]] std::string GetConfigFilenameFromArgs(const int, const char**);
};
}  // namespace core
