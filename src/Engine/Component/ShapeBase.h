#pragma once
#include "Containers/Array.h"
#include "Transform.h"
#include "Graphics/DebugVertex.h"

namespace Seele
{
namespace Component
{
struct ShapeBase
{
    ShapeBase();
    ShapeBase(Array<Vector> vertices, Array<uint32> indices);
    ShapeBase transform(const Component::Transform& transform) const;
    void addCollider(Array<Vector> vertices, Array<uint32> indices, Matrix4 matrix);
    void visualize() const;
    Vector centerOfMass;
    float mass;
    Matrix3 bodyInertia;
    Array<Vector> vertices;
    Array<uint32> indices;
};
} // namespace Component
} // namespace Seele
