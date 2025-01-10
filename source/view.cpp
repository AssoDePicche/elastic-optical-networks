#include "view.h"

Object::Object(const Point2D& point, const draw_fn& fn)
    : point{point}, fn{fn} {}

auto Object::draw(void) const -> void { fn(*this); }
