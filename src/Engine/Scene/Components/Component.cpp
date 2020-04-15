#include "Component.h"

using namespace Seele;

Component::Component()
{
}
Component::~Component()
{
}
PComponent Component::getParent()
{
    
}
PActor Component::getOwner()
{

}
void Component::setWorldLocation(Vector location)
{
}
void Component::setWorldRotation(Vector rotation)
{
}
void Component::setWorldRotation(Quaternion rotation)
{
}
void Component::setWorldScale(Vector scale)
{
}
void Component::setRelativeLocation(Vector location)
{
}
void Component::setRelativeRotation(Vector rotation)
{
}
void Component::setRelativeRotation(Quaternion rotation)
{
}
void Component::setRelativeScale(Vector scale)
{
}
void Component::addWorldTranslation(Vector translation)
{
}
void Component::addWorldRotation(Vector rotation)
{
}
void Component::addWorldRotation(Quaternion rotation)
{
}
void Component::addWorldScale(Vector scale)
{
}
Transform Component::getTransform() const
{
    return transform;
}

void Component::internalSetTransform(Vector newLocation, Quaternion newRotation)
{
    if(parent != nullptr)
    {
        Transform parentTransform = parent->getTransform();
        newLocation = parentTransform.inverseTransformPosition(newLocation);
        newRotation = glm::inverse(parentTransform.getRotation()) * newRotation;
    }

    const Vector newRelRotation = toRotator(newRotation);
    if(newLocation != relativeLocation || newRelRotation != relativeLocation)
    {
        relativeLocation = newLocation;
        relativeRotation = newRelRotation;
        updateComponentTransform(newRelRotation);
    }
}

void Component::propagateTransformUpdate() 
{
    for(auto child : children)
    {
        if(child != nullptr)
        {
            child->updateComponentTransform(child->relativeRotation);
        }
    }
}

void Component::updateComponentTransform(Quaternion relativeRotationQuat) 
{
    if(parent != nullptr && !parent->bComponentTransformClean)
    {
        parent->updateComponentTransform(parent->relativeRotation);

        if(bComponentTransformClean)
        {
            return;
        }
    }
    bComponentTransformClean = true;
    Transform newTransform;
    const Transform relTransform(relativeLocation, relativeRotationQuat, relativeScale);
    if(parent != nullptr)
    {
        newTransform = parent->getTransform() * relTransform;
    }
    
}
