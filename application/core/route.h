#pragma once

#include <unordered_set>

#include "graph.h"

namespace core {
using Route = std::pair<std::unordered_set<Vertex>, Cost>;
}  // namespace core
