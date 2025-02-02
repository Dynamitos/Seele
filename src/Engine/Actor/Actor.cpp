#include "Actor.h"
#include "Scene/Scene.h"

using namespace Seele;

Actor::Actor(PScene scene) : Entity(scene) { attachComponent<Component::Transform>(); }

Actor::~Actor() {}

void Actor::setParent(PActor newParent) {
    if (parent != nullptr) {
        parent->removeChild(this);
    }
    parent = newParent;
}
void Actor::addChild(PActor child) {
    children.add(child);
    child->setParent(this);
}
void Actor::removeChild(PActor child) {
    children.remove(child, false);
    child->setParent(nullptr);
}

Component::Transform& Actor::getTransform() { return accessComponent<Component::Transform>(); }
