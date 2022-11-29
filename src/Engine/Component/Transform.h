#pragma once
#include "Math/Transform.h"
#include "Component.h"

namespace Seele
{
namespace Component
{
struct Transform
{
    Math::Vector getPosition() const { return transform.getPosition(); }
    Math::Quaternion getRotation() const { return transform.getRotation(); }
    Math::Vector getScale() const { return transform.getScale(); }

    Math::Vector getForward() const { return transform.getForward(); }
    Math::Vector getUp() const { return transform.getUp(); }
    Math::Vector getRight() const { return transform.getRight(); }

    Math::Matrix4 toMatrix() const { return transform.toMatrix(); }

    bool isDirty() const { return dirty; }
    void clean() { dirty = false; }

    void setPosition(Math::Vector pos);
    void setRotation(Math::Quaternion quat);
    void setScale(Math::Vector scale);

    void setRelativeLocation(Math::Vector location);
    void setRelativeRotation(Math::Quaternion rotation);
    void setRelativeRotation(Math::Vector rotation);
    void setRelativeScale(Math::Vector scale);

    void addRelativeLocation(Math::Vector translation);
    void addRelativeRotation(Math::Quaternion rotation);
    void addRelativeRotation(Math::Vector rotation);
private:
    bool dirty = true;
    Math::Transform transform;
};
DECLARE_COMPONENT(Transform)

} // namespace Component
} // namespace Seele
