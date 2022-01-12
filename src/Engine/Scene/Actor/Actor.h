#pragma once
#include "MinimalEngine.h"
#include "Math/Transform.h"
#include "ThreadPool.h"

namespace Seele
{
DECLARE_REF(Actor)
DECLARE_REF(Component)
DECLARE_REF(Scene)
class Actor
{
public:
    Actor();
    virtual ~Actor();
    virtual void launchStart();
    virtual void start() {}
    Array<Job> launchTick(float deltaTime) const;
    virtual Job tick(float deltaTime) const { co_return; }
    Array<Job> launchUpdate();
    virtual Job update() { co_return; }
    void notifySceneAttach(PScene scene);
    PScene getScene();

    PActor getParent();
    void addChild(PActor child);
    void detachChild(PActor child);
    Array<PActor> getChildren();
    void setWorldLocation(Vector location);
    void setWorldRotation(Quaternion rotation);
    void setWorldRotation(Vector rotation);
    void setWorldScale(Vector scale);

    void setRelativeLocation(Vector location);
    void setRelativeRotation(Quaternion rotation);
    void setRelativeRotation(Vector rotation);
    void setRelativeScale(Vector scale);

    void addWorldTranslation(Vector translation);
    void addWorldRotation(Quaternion rotation);
    void addWorldRotation(Vector rotation);

    PComponent getRootComponent();
    void setRootComponent(PComponent newRoot);

    // Returns a read-only copy of the actors transform
    Transform getTransform() const;

protected:
    void setParent(PActor parent);
    PScene owningScene;
    PActor parent;
    Array<PActor> children;
    PComponent rootComponent;
};
DEFINE_REF(Actor)
} // namespace Seele