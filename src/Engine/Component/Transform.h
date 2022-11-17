#pragma once
#include "Math/Transform.h"
#include "Component.h"

namespace Seele
{
namespace Component
{
struct Transform
{
    Transform() {}
    Transform(const Transform& other) : transform(other.transform) {}
    Transform(Transform&& other) : transform(other.transform) {}
    Transform& operator=(const Transform& other) {
        if (&other != this)
        {
            transform = other.transform;
        }
        return *this;
    }
    Transform& operator=(Transform&& other) {
        if(&other != this)
        {
            transform = std::move(other.transform);
        }
        return *this;
    }
    Math::Vector getPosition() const { return transform.getPosition(); }
    Math::Quaternion getRotation() const { return transform.getRotation(); }
    Math::Vector getScale() const { return transform.getScale(); }

    Math::Vector getForward() const { return transform.getForward(); }
    Math::Vector getUp() const { return transform.getUp(); }
    Math::Vector getRight() const { return transform.getRight(); }

    Math::Matrix4 toMatrix() const { return transform.toMatrix(); }

    void setRelativeLocation(Math::Vector location);
    void setRelativeRotation(Math::Quaternion rotation);
    void setRelativeRotation(Math::Vector rotation);
    void setRelativeScale(Math::Vector scale);

    void addRelativeLocation(Math::Vector translation);
    void addRelativeRotation(Math::Quaternion rotation);
    void addRelativeRotation(Math::Vector rotation);
private:
    Math::Transform transform;
};
DECLARE_COMPONENT(Transform)

} // namespace Component
} // namespace Seele
