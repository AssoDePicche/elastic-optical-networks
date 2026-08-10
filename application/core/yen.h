#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "graph.h"
#include "path.h"

namespace core {
class Yen {
 public:
  Yen(const Graph&);

  ~Yen();

  [[nodiscard]] std::vector<Path> compute(const Vertex, const Vertex,
                                          const uint8_t) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
};  // namespace core
