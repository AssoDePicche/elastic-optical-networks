#include "view.h"

#include <cmath>

[[nodiscard]] auto deg2rad(const float degrees) -> float {
  return degrees * M_PI / 180.0f;
}

[[nodiscard]] auto rad2deg(const float radians) -> float {
  return radians * 180.0f / M_PI;
}

auto euclidean_distance(const Point2D& p, const Point2D& q) -> float {
  const auto& [x1, y1] = p;

  const auto& [x2, y2] = q;

  const auto dx = x1 - x2;

  const auto dy = y1 - y2;

  return std::sqrt(dx * dx + dy * dy);
}

auto draw(const BoundingBox& box) -> void {
  const auto& [point, width, height, hex] = box;

  const auto& [x, y] = point;

  raylib::DrawRectangleLinesEx({x, y, width, width}, 1.0f, raylib::RED);
}

auto collide(const BoundingBox& a, const BoundingBox& b) -> bool {
  const auto& [p1, w1, h1, c1] = a;

  const auto& [p2, w2, h2, c2] = b;

  const auto& [x1, y1] = p1;

  const auto& [x2, y2] = p2;

  const auto x_overlap = x1 < x2 + w2 && x1 + w1 > x2;

  const auto y_overlap = y1 < y2 + h2 && y1 + h1 > y2;

  return x_overlap && y_overlap;
}

Object::Object(const BoundingBox& box, const Point2D& point, const Color& color,
               const draw_fn& fn)
    : box{box}, point{point}, color{color}, fn{fn} {}

auto Object::draw(void) const -> void { fn(*this); }
