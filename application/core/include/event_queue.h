#pragma once

#include <cassert>
#include <optional>
#include <queue>

#include "distribution.h"

enum class Signal { ARRIVAL, DEPARTURE };

template <typename T>
struct Event {
  double time;
  Signal signal;
  T value;

  Event(void) = default;

  Event(const double time, const Signal &signal, const T &value)
      : time{time}, signal{signal}, value{value} {}

  [[nodiscard]] auto operator<(const Event<T> &other) const noexcept -> bool {
    return time > other.time;
  }
};

template <typename T>
class EventQueue {
 public:
  [[nodiscard]] auto push(const T &value, const double now) -> EventQueue<T> & {
    assert(!pushing);

    pushing = true;

    to_push = value;

    time = now;

    return *this;
  }

  auto of_type(const Signal &signal) {
    assert(pushing);

    auto now = time;

    switch (signal) {
      case Signal::ARRIVAL:
        now += PseudoRandomNumberGenerator::Instance()->next("arrival");
        break;
      case Signal::DEPARTURE:
        now += PseudoRandomNumberGenerator::Instance()->next("service");
        break;
    }

    queue.push(Event<T>(now, signal, to_push));

    pushing = false;
  }

  [[nodiscard]] std::optional<Event<T>> top(void) const {
    if (empty()) {
      return std::nullopt;
    }

    return queue.top();
  }

  [[nodiscard]] std::optional<Event<T>> pop(void) {
    if (empty()) {
      return std::nullopt;
    }

    const auto event{queue.top()};

    queue.pop();

    return event;
  }

  [[nodiscard]] bool empty(void) const { return queue.empty(); }

  [[nodiscard]] size_t size(void) const { return queue.size(); }

 private:
  std::priority_queue<Event<T>> queue;
  double time;
  T to_push;
  bool pushing{false};
};
