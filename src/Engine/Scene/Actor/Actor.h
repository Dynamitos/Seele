#pragma once
#include "MinimalEngine.h"
#include "Math/Transform.h"

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
    void launchTick(float deltaTime) const;
    virtual void tick(float) const { }//co_return; }
    void launchUpdate();
    virtual void update() { }//co_return; }
    void notifySceneAttach(PScene scene);
    PScene getScene();

    PActor getParent();
    void addChild(PActor child);
    void removeChild(PActor child);
    Array<PActor> getChildren();
    //void setAbsoluteLocation(Vector location);
    //void setAbsoluteRotation(Quaternion rotation);
    //void setAbsoluteRotation(Vector rotation);
    //void setWorldScale(Vector scale);

    void setRelativeLocation(Vector location);
    void setRelativeRotation(Quaternion rotation);
    void setRelativeRotation(Vector rotation);
    void setRelativeScale(Vector scale);

    //void addAbsoluteTranslation(Vector translation);
    //void addAbsoluteRotation(Quaternion rotation);
    //void addAbsoluteRotation(Vector rotation);

    void addRelativeLocation(Vector translation);
    void addRelativeRotation(Quaternion rotation);
    void addRelativeRotation(Vector rotation);

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