#pragma once

#include <utility>

#include "cost.h"
#include "vertex.h"

namespace core {
using Edge = std::tuple<Vertex, Vertex, Cost>;
}
