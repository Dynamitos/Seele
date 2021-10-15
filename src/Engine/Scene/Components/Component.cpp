#include "Component.h"
#include "Scene/Actor/Actor.h"
#include "Scene/Scene.h"

using namespace Seele;

Component::Component()
{
    relativeLocation = Vector(0);
    relativeRotation = Vector(0);
    relativeScale = Vector(1, 1, 1);
}
Component::~Component()
{
}
void Component::tick(float)
{
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

void Component::notifySceneAttach(PScene scene)
{
    owningScene = scene;
    scene->getSceneUpdater()->registerComponentUpdate(this);
    for (auto child : children)
    {
        child->notifySceneAttach(scene);
    }
}
void Component::setWorldLocation(Vector location)
{
    Vector newRelLocation = location;
    if (parent != nullptr)
    {
        Transform parentToWorld = getParent()->getTransform();
        newRelLocation = parentToWorld.inverseTransformPosition(location);
    }
    setRelativeLocation(newRelLocation);
}
void Component::setWorldRotation(Vector rotation)
{
    Vector newRelRotator = rotation;
    if (parent == nullptr)
    {
        setRelativeRotation(rotation);
    }
    else
    {
        setWorldRotation(toQuaternion(newRelRotator));
    }
}
void Component::setWorldRotation(Quaternion rotation)
{
    Quaternion newRelRotation = getRelativeWorldRotation(rotation);
    setRelativeRotation(newRelRotation);
}
void Component::setWorldScale(Vector scale)
{
    Vector newRelScale = scale;
    if (parent != nullptr)
    {
        Transform parentToWorld = parent->getTransform();
        newRelScale = scale * parentToWorld.getSafeScaleReciprocal(Vector4(parentToWorld.getScale(), 0));
    }
    setRelativeScale(newRelScale);
}
void Component::setRelativeLocation(Vector location)
{
    setRelativeLocationAndRotation(location, relativeRotation);
}
void Component::setRelativeRotation(Vector rotation)
{
    setRelativeLocationAndRotation(relativeLocation, toQuaternion(rotation));
}
void Component::setRelativeRotation(Quaternion rotation)
{
    setRelativeLocationAndRotation(relativeLocation, rotation);
}
void Component::setRelativeScale(Vector scale)
{
    if (scale != relativeScale)
    {
        relativeScale = scale;
        updateComponentTransform(relativeRotation);
    }
}
void Component::addWorldTranslation(Vector translation)
{
    const Vector newWorldLocation = translation + getTransform().getPosition();
    setWorldLocation(newWorldLocation);
}
void Component::addWorldRotation(Vector rotation)
{
    const Quaternion newWorldRotation = toQuaternion(rotation) * getTransform().getRotation();
    setWorldRotation(newWorldRotation);
}
void Component::addWorldRotation(Quaternion rotation)
{
    const Quaternion newWorldRotation = rotation * getTransform().getRotation();
    setWorldRotation(newWorldRotation);
}
Transform Component::getTransform() const
{
    return transform;
}

void Component::setParent(PComponent newParent)
{
    parent = newParent;
}

void Component::setOwner(PActor newOwner) 
{
    owner = newOwner;
}

void Component::internalSetTransform(Vector newLocation, Quaternion newRotation)
{
    if (parent != nullptr)
    {
        Transform parentTransform = parent->getTransform();
        newLocation = parentTransform.inverseTransformPosition(newLocation);
        newRotation = glm::inverse(parentTransform.getRotation()) * newRotation;
    }

    const Vector newRelRotation = toRotator(newRotation);
    if (newLocation != relativeLocation || newRelRotation != relativeLocation)
    {
        relativeLocation = newLocation;
        relativeRotation = newRelRotation;
        updateComponentTransform(newRelRotation);
    }
}

void Component::propagateTransformUpdate()
{
    for (auto child : children)
    {
        child->updateComponentTransform(child->relativeRotation);
    }
}

void Component::updateComponentTransform(Quaternion relativeRotationQuat)
{
    if (parent != nullptr && !parent->bComponentTransformClean)
    {
        parent->updateComponentTransform(parent->relativeRotation);

        if (bComponentTransformClean)
        {
            return;
        }
    }
    bComponentTransformClean = true;
    Transform newTransform;
    const Transform relTransform(relativeLocation, relativeRotationQuat, relativeScale);
    if(parent != nullptr)
    {
        newTransform = relTransform * parent->getTransform();
    }
    else
    {
        newTransform = relTransform;
    }
    bool bHasChanged = !getTransform().equals(newTransform);
    if (bHasChanged)
    {
        transform = newTransform;
        propagateTransformUpdate();
    }
}

Quaternion Component::getRelativeWorldRotation(Quaternion worldRotation)
{
    Quaternion newRelRotation = worldRotation;
    if (parent != nullptr)
    {
        const Transform parentToWorld = parent->getTransform();

        const Quaternion parentToWorldQuat = parentToWorld.getRotation();
        const Quaternion newRelQuat = glm::inverse(parentToWorldQuat) * worldRotation;
        newRelRotation = newRelQuat;
    }
    return newRelRotation;
}

void Component::setRelativeLocationAndRotation(Vector newLocation, Quaternion newRotation)
{
    if (!bComponentTransformClean)
    {
        updateComponentTransform(toQuaternion(relativeRotation));
    }

    const Transform desiredRelTransform(newLocation, newRotation);
    const Transform desiredWorldTransform = desiredRelTransform; // Check for absolutes etc

    internalSetTransform(desiredWorldTransform.getPosition(), desiredWorldTransform.getRotation());
}
