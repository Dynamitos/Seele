#pragma once
#include "MinimalEngine.h"
#include "Transform.h"

namespace Seele
{
DECLARE_REF(Actor);
DECLARE_REF(Component);
class Actor
{
public:
    Actor();
    virtual ~Actor();
    void setParent(PActor parent);
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
    void addWorldScale(Vector scale);
    
    PComponent getRootComponent();
    void setRootComponent(PComponent newRoot);
private:
    PActor parent;
    Array<PActor> children;
    PComponent rootComponent;
    Transform transform;
};
DEFINE_REF(Actor);
}