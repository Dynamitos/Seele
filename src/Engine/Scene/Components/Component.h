#pragma once
#include "MinimalEngine.h"
#include "Math/Transform.h"

namespace Seele
{
DECLARE_REF(Actor);
DECLARE_REF(Scene);
DECLARE_REF(Component);
class Component
{
public:
    Component();
    virtual ~Component();
    void tick(float deltaTime);
    PComponent getParent();
    PActor getOwner();
    virtual void notifySceneAttach(PScene scene);

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

    Transform getTransform() const;

private:
    void setParent(PComponent parent);
    void internalSetTransform(Vector newLocation, Quaternion newRotation);
    void propagateTransformUpdate();
    void updateComponentTransform(Quaternion relativeRotationQuat);
    Quaternion getRelativeWorldRotation(Quaternion worldRotation);
    void setRelativeLocationAndRotation(Vector newLocation, Quaternion newRotation);
    uint8 bComponentTransformClean : 1;
    Vector relativeLocation;
    Vector relativeRotation;
    Vector relativeScale;
    Transform transform;
    PScene owningScene;
    PActor owner;
    PComponent parent;
    Array<PComponent> children;
    friend class Actor;
};
DEFINE_REF(Component)
} // namespace Seele