#include "Actor.h"
#include "Component.h"

using namespace Seele;

Actor::Actor()
{

}
Actor::~Actor()
{

}
void Actor::setParent(PActor newParent)
{
    parent = newParent;
}
void Actor::addChild(PActor child)
{
    children.add(child);
}
void Actor::detachChild(PActor child)
{
    children.remove(children.find(child), false);
}
void Actor::setWorldLocation(Vector location)
{
    rootComponent->setWorldLocation(location);
}
void Actor::setWorldRotation(Vector4 rotation)
{
    rootComponent->setWorldRotation(rotation);
}
void Actor::setWorldScale(Vector scale)
{
    rootComponent->setWorldScale(scale);
}
void Actor::setRelativeLocation(Vector location)
{
    rootComponent->setRelativeLocation(location);
}
void Actor::setRelativeRotation(Vector4 rotation)
{
    rootComponent->setRelativeRotation(rotation);
}
void Actor::setRelativeScale(Vector scale)
{
    rootComponent->setRelativeScale(scale);
}
void Actor::addWorldTranslation(Vector translation)
{
    rootComponent->addWorldTranslation(translation);
}
void Actor::addWorldRotation(Vector4 rotation)
{
    rootComponent->addWorldRotation(rotation);
}
void Actor::addWorldScale(Vector scale)
{
    rootComponent->addWorldScale(scale);
}
PComponent Actor::getRootComponent()
{
    return rootComponent;
}
void Actor::setRootComponent(PComponent newRoot)
{
    rootComponent = newRoot;
}
