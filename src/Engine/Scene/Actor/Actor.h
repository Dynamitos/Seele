#pragma once
#include "MinimalEngine.h"
#include "Math/Transform.h"

namespace Seele
{
DECLARE_REF(Actor);
DECLARE_REF(Component);
DECLARE_REF(Scene);
class Actor
{
public:
    Actor();
    virtual ~Actor();

    void tick(float deltaTime);
    void notifySceneAttach(PScene scene);
    PScene getScene();

    PActor getParent();
    void addChild(PActor child);
    void detachChild(PActor child);
    Array<PActor> getChildren();
    void setWorldLocation(Vector location);
    void setWorldRotation(Vector4 rotation);
    void setWorldScale(Vector scale);

    void setRelativeLocation(Vector location);
    void setRelativeRotation(Vector4 rotation);
    void setRelativeScale(Vector scale);

    void addWorldTranslation(Vector translation);
    void addWorldRotation(Vector4 rotation);

    PComponent getRootComponent();
    void setRootComponent(PComponent newRoot);

protected:
    void setParent(PActor parent);
    PScene owningScene;
    PActor parent;
    Array<PActor> children;
    PComponent rootComponent;
    Transform transform;
};
DEFINE_REF(Actor);
} // namespace Seele