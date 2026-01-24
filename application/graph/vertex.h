#pragma once

#include <limits>
#include <utility>

#include "cost.h"

namespace graph {
using Vertex = uint64_t;

constexpr Vertex NullVertex = std::numeric_limits<uint64_t>::max();

using AdjacentVertex = std::pair<Vertex, Cost>;
}  // namespace graph
