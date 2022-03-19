#include "Transform.h"
#include <glm/gtx/quaternion.hpp>

using namespace Seele;

Transform::Transform()
    : position(Vector4(0, 0, 0, 0)), rotation(Quaternion(1, 0, 0, 0)), scale(Vector4(1, 1, 1, 0))
{
}

Transform::Transform(Transform &&other)
    : position(other.position), rotation(other.rotation), scale(other.scale)
{
    other.position = Vector4(0, 0, 0, 0);
    other.rotation = Quaternion(1, 0, 0, 0);
    other.scale = Vector4(0, 0, 0, 0);
}

Transform::Transform(const Transform &other)
    : position(other.position), rotation(other.rotation), scale(other.scale)
{
}

Transform::Transform(Vector position)
    : position(Vector4(position, 0)), rotation(Quaternion(1, 0, 0, 0)), scale(Vector4(1, 1, 1, 0))
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
Vector Transform::inverseTransformPosition(const Vector &v) const
{
    return (unrotateVector(rotation, v - Vector(position))) * getSafeScaleReciprocal(scale);
}
Matrix4 Transform::toMatrix()
{
    Matrix4 mat;

    mat[3][0] = position.x;
    mat[3][1] = position.y;
    mat[3][2] = position.z;

    const float x2 = rotation.x + rotation.x;
    const float y2 = rotation.y + rotation.y;
    const float z2 = rotation.z + rotation.z;
    {
        const float xx2 = rotation.x * x2;
        const float yy2 = rotation.y * y2;
        const float zz2 = rotation.z * z2;

        mat[0][0] = (1.0f - (yy2 + zz2)) * scale.x;
        mat[1][1] = (1.0f - (xx2 + zz2)) * scale.y;
        mat[2][2] = (1.0f - (xx2 + yy2)) * scale.z;
    }

    {
        const float yz2 = rotation.y * z2;
        const float wx2 = rotation.w * x2;

        mat[2][1] = (yz2 - wx2) * scale.z;
        mat[1][2] = (yz2 + wx2) * scale.y;
    }
    {
        const float xy2 = rotation.x * y2;
        const float wz2 = rotation.w * z2;

        mat[1][0] = (xy2 - wz2) * scale.y;
        mat[0][1] = (xy2 + wz2) * scale.x;
    }
    {
        const float xz2 = rotation.x * z2;
        const float wy2 = rotation.w * y2;

        mat[2][0] = (xz2 + wy2) * scale.z;
        mat[0][2] = (xz2 - wy2) * scale.x;
    }

    mat[0][3] = 0.0f;
    mat[1][3] = 0.0f;
    mat[2][3] = 0.0f;
    mat[3][3] = 1.0f;

    return mat;
}
Vector Transform::getSafeScaleReciprocal(const Vector4 &inScale, float tolerance)
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

Vector Transform::transformPosition(const Vector &v) const
{
    return Vector(glm::toMat4(rotation) * Vector4(v, 1));
}

Vector Transform::getPosition() const
{
    return Vector(position);
}

Quaternion Transform::getRotation() const
{
    return rotation;
}

Vector Transform::getScale() const
{
    return Vector(scale);
}

Vector Transform::getForward() const
{
    return Vector(0, 0, 1) * rotation;
}
Vector Transform::getRight() const
{
    return Vector(1, 0, 0) * rotation;
}
Vector Transform::getUp() const
{
    return Vector(0, 1, 0) * rotation;
}

bool Transform::equals(const Transform &other, float tolerance)
{
    if (abs(position - other.position).length() > tolerance)
    {
        return false;
    }
    if ((abs(rotation.x - other.rotation.x) <= tolerance 
      && abs(rotation.y - other.rotation.y) <= tolerance 
      && abs(rotation.z - other.rotation.z) <= tolerance 
      && abs(rotation.w - other.rotation.w) <= tolerance)
     || (abs(rotation.x + other.rotation.x) <= tolerance 
      && abs(rotation.y + other.rotation.y) <= tolerance 
      && abs(rotation.z + other.rotation.z) <= tolerance 
      && abs(rotation.w + other.rotation.w) <= tolerance))
    {
        return false;
    }
    if (abs(scale - other.scale).length() > tolerance)
    {
        return false;
    }
    return true;
}

void Transform::multiply(Transform *outTransform, const Transform *a, const Transform *b)
{
    outTransform->rotation = b->rotation * a->rotation;
    outTransform->position = b->rotation * (b->scale * a->position) + b->position;
    outTransform->scale = b->scale * a->scale;
}
void Transform::add(Transform *outTransform, const Transform *a, const Transform *b)
{
    outTransform->position = a->position + b->position;
    outTransform->rotation = a->rotation + b->rotation;
    outTransform->scale = a->scale + b->scale;
}


Transform &Transform::operator=(const Transform &other)
{
    position = other.position;
    rotation = other.rotation;
    scale = other.scale;
    return *this;
}

Transform &Transform::operator=(Transform &&other)
{
    position = other.position;
    rotation = other.rotation;
    scale = other.scale;
    other.position = Vector4(0, 0, 0, 0);
    other.rotation = Quaternion(1, 0, 0, 0);
    other.scale = Vector4(0, 0, 0, 0);
    return *this;
}

Transform Transform::operator+(const Transform & other) const
{
    Transform outTransform;
    add(&outTransform, this, &other);
    return outTransform;
}

Transform Transform::operator*(const Transform &other) const
{
    Transform outTransform;
    multiply(&outTransform, this, &other);
    return outTransform;
}
