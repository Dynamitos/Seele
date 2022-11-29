#pragma once
#include "Containers/Array.h"
#include "Transform.h"

namespace Seele
{
namespace Component
{
struct ShapeBase
{
    ShapeBase(Array<Math::Vector> vertices, Array<uint32> indices);
    ShapeBase transform(const Component::Transform& transform) const;
    void addCollider(Array<Math::Vector> vertices, Array<uint32> indices, Math::Matrix4 matrix);
    Math::Vector centerOfMass;
    float mass;
    Math::Matrix3 bodyInertia;
    Array<Math::Vector> vertices;
    Array<uint32> indices;
};
} // namespace Component
} // namespace Seele
