#pragma once

namespace graph {
struct Cost {
  double value;

  Cost(void) = default;

  Cost(double);

  static Cost max(void);

  static Cost min(void);
};

[[nodiscard]] bool operator<(const Cost&, const Cost&);
}  // namespace graph
