#include "path.h"

namespace core {
struct Path::Implementation {
  std::vector<Vertex> vertices;
  double cost;
};
};  // namespace core
