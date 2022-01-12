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

void Actor::launchStart() 
{
    rootComponent->launchStart();
    for(auto child : children)
    {
        child->launchStart();
    }
    start();
}
Array<Job> Actor::launchTick(float deltaTime) const
{
    Array<Job> result = rootComponent->launchTick(deltaTime);
    for(auto child : children)
    {
        result.addAll(child->launchTick(deltaTime));
    }
    result.add(tick(deltaTime));
    return result;
}

Array<Job> Actor::launchUpdate() 
{
    Array<Job> result = rootComponent->launchUpdate();
    for(auto child : children)
    {
        result.addAll(child->launchUpdate());
    }
    result.add(update());
    return result;
}
void Actor::notifySceneAttach(PScene scene)
{
    owningScene = scene;
    rootComponent->notifySceneAttach(scene);
    for(auto child : children)
    {
        child->notifySceneAttach(scene);
    }
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

void Actor::setWorldRotation(Quaternion rotation) 
{
    rootComponent->setWorldRotation(rotation);
}
void Actor::setWorldRotation(Vector rotation)
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
void Actor::setRelativeRotation(Quaternion rotation)
{
    rootComponent->setRelativeRotation(rotation);
}
void Actor::setRelativeRotation(Vector rotation)
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
void Actor::addWorldRotation(Quaternion rotation)
{
    rootComponent->addWorldRotation(rotation);
}
void Actor::addWorldRotation(Vector rotation)
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

Transform Actor::getTransform() const
{
    return rootComponent->getTransform();
}
