#include "Transform.h"

using namespace Seele::Component;

DEFINE_COMPONENT(Transform)


void Transform::setPosition(Vector pos)
{
    transform.setPosition(pos);
}

void Transform::setRotation(Quaternion quat)
{
    transform.setRotation(quat);
}

void Transform::setScale(Vector scale)
{
    transform.setScale(scale);
}

void Transform::setRelativeLocation(Vector location)
{
    transform = Math::Transform(location, transform.getRotation(), transform.getScale());
}
void Transform::setRelativeRotation(Vector rotation)
{
    transform = Math::Transform(transform.getPosition(), Quaternion(rotation), transform.getScale());
}
void Transform::setRelativeRotation(Quaternion rotation)
{
    transform = Math::Transform(transform.getPosition(), rotation, transform.getScale());
}
void Transform::setRelativeScale(Vector scale)
{
    transform = Math::Transform(transform.getPosition(), transform.getRotation(), scale);
}
void Transform::addRelativeLocation(Vector translation)
{
    transform = Math::Transform(transform.getPosition() + translation, transform.getRotation(), transform.getScale());
}
void Transform::addRelativeRotation(Vector rotation)
{
    transform = Math::Transform(transform.getPosition(), transform.getRotation() * Quaternion(rotation), transform.getScale());
}
void Transform::addRelativeRotation(Quaternion rotation)
{
    transform = Math::Transform(transform.getPosition(), transform.getRotation() * rotation, transform.getScale());
}