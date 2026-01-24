#include "cost.h"

#include <limits>

namespace graph {
Cost::Cost(double value) : value{value} {}

Cost Cost::max(void) { return Cost(std::numeric_limits<double>::max()); }

Cost Cost::min(void) { return Cost(.0f); }

bool operator<(const Cost& lhs, const Cost& rhs) {
  return lhs.value < rhs.value;
}
}  // namespace graph
