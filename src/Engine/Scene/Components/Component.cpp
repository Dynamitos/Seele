#include "Component.h"
#include "Scene/Actor/Actor.h"
#include "Scene/Scene.h"

using namespace Seele;

Component::Component()
{
}
Component::~Component()
{
}

void Component::launchStart()
{
    for(auto child : children)
    {
        child->launchStart();
    }
    start();
}

void Component::launchTick(float deltaTime) const
{
    for(auto child : children)
    {
        child->launchTick(deltaTime);
    }
    tick(deltaTime);
}

void Component::launchUpdate()
{
    for(auto child : children)
    {
        child->launchUpdate();
    }
    update();
}

PComponent Component::getParent()
{
    return parent;
}
PActor Component::getOwner()
{
    if (owner != nullptr)
    {
        return owner;
    }
    if (parent != nullptr)
    {
        return parent->getOwner();
    }
    return nullptr;
}
const Array<PComponent>& Component::getChildComponents()
{
    return children;
}

void Component::setParent(PComponent newParent)
{
    parent = newParent;
}

void Component::setOwner(PActor newOwner) 
{
    owner = newOwner;
}

void Component::addChildComponent(PComponent component)
{
    children.add(component);
}

void Component::notifySceneAttach(PScene scene)
{
    owningScene = scene;
    for (auto child : children)
    {
        child->notifySceneAttach(scene);
    }
}

//void Component::setAbsoluteLocation(Vector location)
//{
//    Vector newRelLocation = location;
//    if (parent != nullptr)
//    {
//        Transform parentToWorld = getParent()->getTransform();
//        newRelLocation = parentToWorld.inverseTransformPosition(location);
//    }
//    setRelativeLocation(newRelLocation);
//}
//void Component::setAbsoluteRotation(Vector rotation)
//{
//    Vector newRelRotator = rotation;
//    if (parent == nullptr)
//    {
//        setRelativeRotation(rotation);
//    }
//    else
//    {
//        setAbsoluteRotation(toQuaternion(newRelRotator));
//    }
//}
//void Component::setAbsoluteRotation(Quaternion rotation)
//{
//    Quaternion newRelRotation = getRelativeWorldRotation(rotation);
//    setRelativeRotation(newRelRotation);
//}
//void Component::setWorldScale(Vector scale)
//{
//    Vector newRelScale = scale;
//    if (parent != nullptr)
//    {
//        Transform parentToWorld = parent->getTransform();
//        newRelScale = scale * parentToWorld.getSafeScaleReciprocal(Vector4(parentToWorld.getScale(), 0));
//    }
//    setRelativeScale(newRelScale);
//}

void Component::setRelativeLocation(Vector location)
{
    transform = Transform(location, transform.getRotation(), transform.getScale());
    propagateTransformUpdate();
}
void Component::setRelativeRotation(Vector rotation)
{
    transform = Transform(transform.getPosition(), Quaternion(rotation), transform.getScale());
    propagateTransformUpdate();
}
void Component::setRelativeRotation(Quaternion rotation)
{
    transform = Transform(transform.getPosition(), rotation, transform.getScale());
    propagateTransformUpdate();
}
void Component::setRelativeScale(Vector scale)
{
    transform = Transform(transform.getPosition(), transform.getRotation(), scale);
    propagateTransformUpdate();
}

//void Component::addAbsoluteTranslation(Vector translation)
//{
//    const Vector newWorldLocation = translation + getTransform().getPosition();
//    setAbsoluteLocation(newWorldLocation);
//}
//void Component::addAbsoluteRotation(Vector rotation)
//{
//    const Quaternion newWorldRotation = toQuaternion(rotation) * getTransform().getRotation();
//    setAbsoluteRotation(newWorldRotation);
//}
//void Component::addAbsoluteRotation(Quaternion rotation)
//{
//    const Quaternion newWorldRotation = rotation * getTransform().getRotation();
//    setAbsoluteRotation(newWorldRotation);
//}

void Component::addRelativeLocation(Vector translation)
{
    transform = Transform(transform.getPosition() + translation, transform.getRotation(), transform.getScale());
    propagateTransformUpdate();
}
void Component::addRelativeRotation(Vector rotation)
{
    transform = Transform(transform.getPosition(), transform.getRotation() * Quaternion(rotation), transform.getScale());
    propagateTransformUpdate();
}
void Component::addRelativeRotation(Quaternion rotation)
{
    transform = Transform(transform.getPosition(), transform.getRotation() * rotation, transform.getScale());
    propagateTransformUpdate();
}

Transform Component::getTransform() const
{
    return transform;
}

Transform Component::getAbsoluteTransform() const
{
    return absoluteTransform;
}

void Component::propagateTransformUpdate()
{
    if(parent != nullptr)
    {
        absoluteTransform = transform + parent->getAbsoluteTransform();
    }
    else
    {
        absoluteTransform = transform;
    }
    for(auto child : children)
    {
        child->propagateTransformUpdate();
    }
}