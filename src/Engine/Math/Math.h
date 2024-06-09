#pragma once
#include "EngineTypes.h"
#include "Matrix.h"
#include "Vector.h"


namespace Seele {
struct Rect {
    bool isEmpty() const { return size.x == 0 || size.y == 0; }
    Vector2 size;
    Vector2 offset;
};
// Unsigned int
struct URect {
    UVector2 size = UVector2(0);
    UVector2 offset = UVector2(0);
};
struct Rect3D {
    Vector size = Vector(0);
    Vector offset = Vector(0);
};
} // namespace Seele