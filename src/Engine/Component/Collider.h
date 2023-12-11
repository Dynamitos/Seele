#pragma once
#include "Math/AABB.h"
#include "ShapeBase.h"

namespace Seele
{
namespace Component
{
enum class ColliderType
{
    STATIC,
    DYNAMIC,
};
struct Collider
{
    ColliderType type = ColliderType::STATIC;
    AABB boundingbox;
    ShapeBase physicsMesh;
    Collider transform(const Transform& transform) const;
};
} // namespace Component
} // namespace Seele
