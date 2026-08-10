#pragma once

#include <cstdint>
#include <utility>

#include "cost.h"

namespace core {
using Vertex = uint64_t;

using AdjacentVertex = std::pair<Vertex, Cost>;
}  // namespace core
