#pragma once
#include "MinimalEngine.h"
#include "Math/Transform.h"
#include "Scene/Util.h"
#include "ThreadPool.h"

namespace Seele
{
DECLARE_REF(Actor)
DECLARE_REF(Scene)
DECLARE_REF(Component)
class Component
{
public:
    Component();
    virtual ~Component();
    void launchStart();
    virtual void start() {};
    Array<Job> launchTick(float deltaTime) const;
    virtual Job tick(float deltaTime) const { co_return; }
    Array<Job> launchUpdate();
    virtual Job update() { co_return; }
    PComponent getParent();
    PActor getOwner();
    const Array<PComponent>& getChildComponents();
    void setParent(PComponent parent);
    void setOwner(PActor owner);
    void addChildComponent(PComponent component);
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

    template<typename ComponentType>
    RefPtr<ComponentType> getComponent()
    {
        return tryFindComponent<ComponentType>();
    }
private:
    template<typename ComponentType>
    RefPtr<ComponentType> tryFindComponent()
    {
        for(auto child : children)
        {
            RefPtr<ComponentType> result = child.cast<ComponentType>();
            if(result != nullptr)
            {
                return result;
            }
            result = parent->tryFindComponent<ComponentType>();
            if(result != nullptr)
            {
                return result;
            }
        }
        return nullptr;
    }
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