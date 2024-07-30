#pragma once

#include <algorithm>
#include <cassert>
#include <optional>
#include <queue>
#include <vector>

#include "distribution.h"

using Time = double;

enum class Signal { ARRIVAL, DEPARTURE };

template <typename T> struct Event {
  Time time;
  Signal signal;
  T value;

  Event(void) = default;

  Event(const Time time, const Signal &signal, const T &value)
      : time{time}, signal{signal}, value{value} {}
};

template <typename T>
[[nodiscard]] auto operator<(const Event<T> &a, const Event<T> &b) -> bool {
  return a.time < b.time;
}

template <typename T>
[[nodiscard]] auto operator>(const Event<T> &a, const Event<T> &b) -> bool {
  return a.time > b.time;
}

template <typename T> class EventQueue {
public:
  EventQueue(const double arrival_rate, const double service_rate)
      : arrival{arrival_rate}, service{service_rate} {}

  [[nodiscard]] auto push(const T &value, const Time now) -> EventQueue<T> & {
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
      now += (1.0 / arrival.next());
      break;
    case Signal::DEPARTURE:
      now += service.next();
      break;
    }

    queue.push(Event<T>(now, signal, to_push));

    pushing = false;
  }

  [[nodiscard]] auto pop(void) -> std::optional<Event<T>> {
    if (empty()) {
      return std::nullopt;
    }

    const auto event = queue.top();

    queue.pop();

    return event;
  }

  [[nodiscard]] auto empty(void) const -> bool { return queue.empty(); }

  [[nodiscard]] auto size(void) const -> std::size_t { return queue.size(); }

private:
  std::priority_queue<Event<T>, std::vector<Event<T>>, std::greater<Event<T>>>
      queue;
  Exponential arrival;
  Exponential service;
  Time time;
  T to_push;
  bool pushing{false};
};
