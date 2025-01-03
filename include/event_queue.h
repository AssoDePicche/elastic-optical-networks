#pragma once

#include <algorithm>
#include <cassert>
#include <memory>
#include <optional>
#include <queue>
#include <random>
#include <vector>

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
  EventQueue(const double arrival_rate, const double service_rate,
             const Seed seed)
      : arrival{seed, arrival_rate}, service{seed, service_rate} {}

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
        now += arrival.next();
        break;
      case Signal::DEPARTURE:
        now += service.next();
        break;
    }

    queue.push(Event<T>(now, signal, to_push));

    pushing = false;
  }

  [[nodiscard]] auto top(void) const -> std::optional<Event<T>> {
    if (empty()) {
      return std::nullopt;
    }

    return queue.top();
  }

  [[nodiscard]] auto pop(void) -> std::optional<Event<T>> {
    if (empty()) {
      return std::nullopt;
    }

    const auto event{queue.top()};

    queue.pop();

    return event;
  }

  [[nodiscard]] auto empty(void) const -> bool { return queue.empty(); }

  [[nodiscard]] auto size(void) const -> std::size_t { return queue.size(); }

 private:
  std::priority_queue<Event<T>> queue;
  Exponential arrival;
  Exponential service;
  double time;
  T to_push;
  bool pushing{false};
};
