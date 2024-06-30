#pragma once

#include <chrono>

class Timer final {
public:
  template <typename T> [[nodiscard]] auto elapsed(void) -> long;

  auto start(void) -> void;

  auto stop(void) -> void;

private:
  std::chrono::time_point<std::chrono::system_clock> beginning;
  std::chrono::time_point<std::chrono::system_clock> end;
  bool running{false};

  [[nodiscard]] auto now(void)
      -> std::chrono::time_point<std::chrono::system_clock>;
};
