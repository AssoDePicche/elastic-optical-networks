#pragma once

#include <functional>
#include <utility>

namespace raylib {
#include <raylib.h>
};

using Color = unsigned int;

using Point2D = std::pair<float, float>;

[[nodiscard]] auto euclidean_distance(const Point2D&, const Point2D&) -> float;

using BoundingBox = std::tuple<Point2D, float, float, Color>;

[[nodiscard]] auto collide(const BoundingBox&, const BoundingBox&) -> bool;

struct Object;

using draw_fn = std::function<void(const Object&)>;

struct Object final {
  BoundingBox box;
  Point2D point;
  Color color;
  draw_fn fn;

  Object(void) = default;

  Object(const BoundingBox&, const Point2D&, const Color&, const draw_fn&);

  auto draw(void) const -> void;
};
