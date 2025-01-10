#include "view.h"

#include <cmath>

auto euclidean_distance(const Point2D& p, const Point2D& q) -> float {
  const auto& [x1, y1] = p;

  const auto& [x2, y2] = q;

  const auto dx = x1 - x2;

  const auto dy = y1 - y2;

  return std::sqrt(dx * dx + dy * dy);
}

Object::Object(const Point2D& point, const draw_fn& fn)
    : point{point}, fn{fn} {}

auto Object::draw(void) const -> void { fn(*this); }
