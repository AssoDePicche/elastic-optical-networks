#pragma once

#include <initializer_list>
#include <tuple>

#include "distribution.h"

class Group final {
public:
  Group(const Seed seed, const std::initializer_list<double> &,
        const std::initializer_list<std::size_t> &);

  auto count_blocking(const std::size_t) -> void;

  [[nodiscard]] auto size(void) const -> std::size_t;

  [[nodiscard]] auto blocked(void) const -> std::size_t;

  [[nodiscard]] auto next(void) -> std::size_t;

  [[nodiscard]] auto to_string(void) const -> std::string;

private:
  Discrete roulette;
  std::vector<std::tuple<std::size_t, std::size_t, std::size_t>> container;
  std::size_t _size;
  std::size_t _blocking;
};
