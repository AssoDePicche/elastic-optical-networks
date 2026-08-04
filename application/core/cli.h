#pragma once

#include <memory>

namespace core {
class CommandLineInterface final {
 public:
  CommandLineInterface();

  ~CommandLineInterface();

  [[nodiscard]] int Run(const int, const char**);

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
