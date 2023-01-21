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

    bool isDirty() const { return dirty; }
    void clean() { dirty = false; }

    void setPosition(Vector pos);
    void setRotation(Quaternion quat);
    void setScale(Vector scale);

    void setRelativeLocation(Vector location);
    void setRelativeRotation(Quaternion rotation);
    void setRelativeRotation(Vector rotation);
    void setRelativeScale(Vector scale);

    void addRelativeLocation(Vector translation);
    void addRelativeRotation(Quaternion rotation);
    void addRelativeRotation(Vector rotation);
private:
    bool dirty = true;
    Math::Transform transform;
};
DECLARE_COMPONENT(Transform)

} // namespace Component
} // namespace Seele
