#pragma once

#include <functional>
#include <utility>

namespace raylib {
#include <raylib.h>
};

using Point2D = std::pair<float, float>;

struct Object;

using draw_fn = std::function<void(const Object&)>;

struct Object final {
  Point2D point;
  draw_fn fn;

  Object(void) = default;

  Object(const Point2D&, const draw_fn&);

  auto draw(void) const -> void;
};
