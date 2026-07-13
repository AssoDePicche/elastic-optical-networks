#pragma once

namespace graph {
struct Cost {
  double value;

  Cost(void) = default;

  Cost(double);

  auto operator<=>(const Cost&) const = default;

  bool operator==(const Cost&) const = default;

  static Cost max(void);

  static Cost min(void);
};

[[nodiscard]] bool operator<(const Cost&, const Cost&);
}  // namespace graph
