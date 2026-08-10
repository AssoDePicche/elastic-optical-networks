#pragma once

#include <memory>
#include <optional>
#include <unordered_map>

#include "graph.h"
#include "route.h"

namespace core {
class Router final {
 public:
  Router(const Graph&);

  ~Router();

  [[nodiscard]] Route compute(void) const;

  [[nodiscard]] std::optional<Route> compute(const Vertex, const Vertex) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
