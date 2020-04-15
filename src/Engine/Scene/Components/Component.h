#pragma once
#include "MinimalEngine.h"
#include "Transform.h"

namespace Seele
{
DECLARE_REF(Actor);
class Component
{
public:
    Component();
    virtual ~Component();
    PComponent getParent();
    void setParent(PComponent parent);
    PActor getOwner();
    void setOwner(PActor owner);

    void setWorldLocation(Vector location);
    void setWorldRotation(Vector rotation);
    void setWorldRotation(Quaternion rotation);
    void setWorldScale(Vector scale);

    void setRelativeLocation(Vector location);
    void setRelativeRotation(Vector rotation);
    void setRelativeRotation(Quaternion rotation);
    void setRelativeScale(Vector scale);
    
    void addWorldTranslation(Vector translation);
    void addWorldRotation(Vector rotation);
    void addWorldRotation(Quaternion rotation);
    void addWorldScale(Vector scale);

    Transform getTransform() const;
private:
    void internalSetTransform(Vector newLocation, Quaternion newRotation);
    void propagateTransformUpdate();
    void updateComponentTransform(Quaternion relativeRotationQuat);
    uint8 bComponentTransformClean:1;
    Vector relativeLocation;
    Vector relativeRotation;
    Vector relativeScale;
    Transform transform;
    PActor owner;
    PComponent parent;
    Array<PComponent> children;
};
DEFINE_REF(Component)
}