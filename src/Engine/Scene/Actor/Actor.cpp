#include "Actor.h"
#include "Scene/Scene.h"

using namespace Seele;

Actor::Actor(PScene scene)
    : Entity(scene)
{
    scene->attachComponent<Component::Transform>(identifier);
}

Actor::~Actor()
{

}

void Actor::setParent(PActor newParent)
{
    if(parent != nullptr)
    {
        parent->removeChild(this);
    }
    parent = newParent;
}
void Actor::addChild(PActor child)
{
    children.add(child);
    child->setParent(this);
}
void Actor::removeChild(PActor child)
{
    children.remove(children.find(child), false);
    child->setParent(nullptr);
}
const Component::Transform& Actor::getTransform() const
{
    return scene->accessComponent<Component::Transform>(identifier);
}

//Component::Transform& Actor::getTransform()
//{
//    return scene->accessComponent<Component::Transform>(identifier);
//}

