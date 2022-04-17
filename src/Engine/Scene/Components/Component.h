#pragma once
#include "MinimalEngine.h"
#include "Math/Transform.h"
#include "Scene/Util.h"

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
    void launchTick(float deltaTime) const;
    virtual void tick(float) const { }//co_return; }
    void launchUpdate();
    virtual void update() { }//co_return; }
    PComponent getParent();
    PActor getOwner();
    const Array<PComponent>& getChildComponents();
    void setParent(PComponent parent);
    void setOwner(PActor owner);
    void addChildComponent(PComponent component);
    virtual void notifySceneAttach(PScene scene);

    void setRelativeLocation(Vector location);
    void setRelativeRotation(Vector rotation);
    void setRelativeRotation(Quaternion rotation);
    void setRelativeScale(Vector scale);

    void addRelativeLocation(Vector translation);
    void addRelativeRotation(Vector rotation);
    void addRelativeRotation(Quaternion rotation);

    Transform getTransform() const;
    Transform getAbsoluteTransform() const;

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
    void propagateTransformUpdate();
    Transform transform;
    Transform absoluteTransform;
    PScene owningScene;
    PActor owner;
    PComponent parent;
    Array<PComponent> children;
    friend class Actor;
};
DEFINE_REF(Component)
} // namespace Seele