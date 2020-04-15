#include "Transform.h"

using namespace Seele;

Transform::Transform()
    : position(Vector4(0, 0, 0, 0)), rotation(Quaternion(0, 0, 0, 0)), scale(Vector4(1, 1, 1, 0))
{
}

Transform::Transform(Vector position)
    : position(Vector4(position, 0)), rotation(Quaternion(0, 0, 0, 0)), scale(Vector4(1, 1, 1, 0))
{
}
Transform::Transform(Vector position, Quaternion rotation)
    : position(Vector4(position, 0)), rotation(rotation), scale(Vector4(1, 1, 1, 0))
{
}
Transform::Transform(Vector position, Quaternion rotation, Vector scale)
    : position(Vector4(position, 0)), rotation(rotation), scale(Vector4(scale, 0))
{
}
Transform::Transform(Quaternion rotation, Vector scale)
    : position(Vector4(0, 0, 0, 0)), rotation(rotation), scale(Vector4(scale, 0))
{
}
Transform::~Transform()
{
}
Vector Transform::inverseTransformPosition(const Vector& v) const
{
    return (unrotateVector(rotation, v - Vector(position))) * getSafeScaleReciprocal(scale);
}

Vector Transform::getSafeScaleReciprocal(const Vector4& inScale, float tolerance) 
{
    Vector safeReciprocalScale;
    if (abs(inScale.x) <= tolerance)
    {
        safeReciprocalScale.x = 0.f;
    }
    else
    {
        safeReciprocalScale.x = 1 / inScale.x;
    }
    if (abs(inScale.y) <= tolerance)
    {
        safeReciprocalScale.y = 0.f;
    }
    else
    {
        safeReciprocalScale.y = 1 / inScale.y;
    }
    if (abs(inScale.z) <= tolerance)
    {
        safeReciprocalScale.z = 0.f;
    }
    else
    {
        safeReciprocalScale.z = 1 / inScale.z;
    }
    return safeReciprocalScale;
}

inline Vector Transform::getPosition() const
{
    return Vector(position);
}

inline Quaternion Transform::getRotation() const
{
    return rotation;
}

inline Vector Transform::getScale() const
{
    return Vector(scale);
}

inline void Transform::multiply(Transform* outTransform, const Transform* a, const Transform* b) 
{
    outTransform->rotation = b->rotation * a->rotation;
    outTransform->position = b->position * (b->scale * a->position) + b->position;
    outTransform->scale = b->scale * a->scale;
}
