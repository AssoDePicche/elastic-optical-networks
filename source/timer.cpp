#include "timer.h"

template <typename T> auto Timer::elapsed(void) -> long {
  if (running) {
    end = now();
  }

  return std::chrono::duration_cast<T>(end - beginning).count();
}

auto Timer::start(void) -> void {
  beginning = now();

  running = true;
}

auto Timer::stop(void) -> void {
  end = now();

  running = false;
}

auto Timer::now(void) -> std::chrono::time_point<std::chrono::system_clock> {
  return std::chrono::system_clock::now();
}
