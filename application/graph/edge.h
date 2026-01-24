#pragma once

#include <utility>

#include "cost.h"
#include "vertex.h"

namespace graph {
using Edge = std::tuple<Vertex, Vertex, Cost>;
}
