#include "Actor.h"
#include "Scene/Components/Component.h"
#include "Scene/Scene.h"

using namespace Seele;

Actor::Actor()
{

}
Actor::~Actor()
{

}
void Actor::tick(float deltaTime)
{
    rootComponent->tick(deltaTime);
    for(auto child : children)
    {
        child->tick(deltaTime);
    }
}
void Actor::notifySceneAttach(PScene scene)
{
    owningScene = scene;
    rootComponent->notifySceneAttach(scene);
}

PScene Actor::getScene()
{
    return owningScene;
}

void Actor::setParent(PActor newParent)
{
    parent = newParent;
}
void Actor::addChild(PActor child)
{
    children.add(child);
    child->setParent(this);
    child->notifySceneAttach(owningScene);
}
void Actor::detachChild(PActor child)
{
    children.remove(children.find(child), false);
    child->setParent(nullptr);
    child->notifySceneAttach(nullptr);
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
PComponent Actor::getRootComponent()
{
    return rootComponent;
}
void Actor::setRootComponent(PComponent newRoot)
{
    if(rootComponent != nullptr)
    {
        rootComponent->owner = nullptr;
    }
    rootComponent = newRoot;
    rootComponent->owner = this;
}
