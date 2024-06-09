#include "Transform.h"

using namespace Seele::Component;

DEFINE_COMPONENT(Transform);

void Transform::setPosition(Vector pos) { transform.setPosition(pos); }

void Transform::setRotation(Quaternion quat) { transform.setRotation(quat); }

void Transform::setScale(Vector scale) { transform.setScale(scale); }
void Transform::translate(Vector direction) { transform.setPosition(transform.getPosition() + direction); }
void Transform::rotate(Quaternion quat) { transform.setRotation(transform.getRotation() * quat); }
void Transform::scale(Vector scale) { transform.setScale(transform.getScale() + scale); }