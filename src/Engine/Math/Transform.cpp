#include "Transform.h"
#include <glm/gtx/quaternion.hpp>

using namespace Seele;
using namespace Seele::Math;

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
Matrix4 Transform::toMatrix() const
{
    // TODO: actual calculations, SIMD
    Matrix4 result = Matrix4(1);
    result = glm::scale(result, Vector(scale));
    result = glm::toMat4(rotation) * result;
    result[3][0] = position.x;
    result[3][1] = position.y;
    result[3][2] = position.z;
    return result;
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

void Transform::setPosition(Vector pos)
{
    position = Vector4(pos, 0);
}

void Transform::setRotation(Quaternion quat)
{
    rotation = quat;
}

void Transform::setScale(Vector s)
{
    scale = Vector4(s, 0);
}

Vector Transform::getForward() const
{
    return glm::normalize(Vector(0, 0, 1) * rotation);
}

Vector Transform::getRight() const
{
    return glm::normalize(Vector(1, 0, 0) * rotation);
}

Vector Transform::getUp() const
{
    return glm::normalize(Vector(0, 1, 0) * rotation);
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
    if(&other != this)
    {
        position = other.position;
        rotation = other.rotation;
        scale = other.scale;
    }
    return *this;
}

Transform &Transform::operator=(Transform &&other)
{
    if(&other != this)
    {
        position = other.position;
        rotation = other.rotation;
        scale = other.scale;
        other.position = Vector4(0, 0, 0, 0);
        other.rotation = Quaternion(1, 0, 0, 0);
        other.scale = Vector4(0, 0, 0, 0);
    }
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
