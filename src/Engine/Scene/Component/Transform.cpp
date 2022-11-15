#include "Transform.h"

using namespace Seele::Component;

DEFINE_COMPONENT(Transform)

void Transform::setRelativeLocation(Math::Vector location)
{
    transform = Math::Transform(location, transform.getRotation(), transform.getScale());
}
void Transform::setRelativeRotation(Math::Vector rotation)
{
    transform = Math::Transform(transform.getPosition(), Math::Quaternion(rotation), transform.getScale());
}
void Transform::setRelativeRotation(Math::Quaternion rotation)
{
    transform = Math::Transform(transform.getPosition(), rotation, transform.getScale());
}
void Transform::setRelativeScale(Math::Vector scale)
{
    transform = Math::Transform(transform.getPosition(), transform.getRotation(), scale);
}
void Transform::addRelativeLocation(Math::Vector translation)
{
    transform = Math::Transform(transform.getPosition() + translation, transform.getRotation(), transform.getScale());
}
void Transform::addRelativeRotation(Math::Vector rotation)
{
    transform = Math::Transform(transform.getPosition(), transform.getRotation() * Math::Quaternion(rotation), transform.getScale());
}
void Transform::addRelativeRotation(Math::Quaternion rotation)
{
    transform = Math::Transform(transform.getPosition(), transform.getRotation() * rotation, transform.getScale());
}