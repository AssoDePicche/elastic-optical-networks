#pragma once

#include <memory>

#include "graph.h"
#include "path.h"

namespace core {
class Router final {
 public:
  Router(const Graph&);

  ~Router();

  [[nodiscard]] Path compute(void) const;

 private:
  struct Implementation;
  std::unique_ptr<Implementation> pImpl;
};
}  // namespace core
