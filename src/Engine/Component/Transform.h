#pragma once
#include "Math/Transform.h"
#include "Component.h"

namespace Seele
{
namespace Component
{
struct Transform
{
    Vector getPosition() const { return transform.getPosition(); }
    Quaternion getRotation() const { return transform.getRotation(); }
    Vector getScale() const { return transform.getScale(); }

    Vector getForward() const { return transform.getForward(); }
    Vector getUp() const { return transform.getUp(); }
    Vector getRight() const { return transform.getRight(); }

    Matrix4 toMatrix() const { return transform.toMatrix(); }

    void setPosition(Vector pos);
    void setRotation(Quaternion quat);
    void setScale(Vector scale);
    void translate(Vector direction);
    void rotate(Quaternion quat);
    void scale(Vector scale);
private:
    Math::Transform transform;
};
DECLARE_COMPONENT(Transform)

} // namespace Component
} // namespace Seele
