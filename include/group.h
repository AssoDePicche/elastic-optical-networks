#pragma once

#include <initializer_list>
#include <tuple>

#include "distribution.h"
#include "settings.h"

class Group final {
 public:
  Group(const Seed seed, const std::initializer_list<double> &,
        const std::initializer_list<unsigned> &);

  auto count_blocking(const unsigned) -> void;

  [[nodiscard]] auto size(void) const -> unsigned;

  [[nodiscard]] auto blocked(void) const -> unsigned;

  [[nodiscard]] auto blocking(void) const -> double;

  [[nodiscard]] auto next(void) -> unsigned;

  [[nodiscard]] auto to_string(void) const -> std::string;

 private:
  Discrete roulette;
  std::vector<std::tuple<unsigned, unsigned, unsigned>> container;
  unsigned _size{};
  unsigned _blocking{};
};

struct Report {
  std::string group_str;
  unsigned long long seed;
  unsigned bandwidth;
  unsigned requests;
  double arrival_rate;
  double service_rate;
  double traffic_intensity;
  double busy_channels;
  double grade_of_service;
  double occupancy;

  [[nodiscard]] static auto from(const Group &, const Settings &) -> Report;

  [[nodiscard]] auto to_string(void) const -> std::string;
};
