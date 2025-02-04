#pragma once
#include "Component/Transform.h"
#include "Entity.h"


namespace Seele {
DECLARE_REF(Actor)
// Actors are entities that are part of the scene hierarchy
// In order for that hierarchy to make sense it requires at least a transform component
class Actor : public Entity {
  public:
    Actor(PScene scene);
    virtual ~Actor();

    PActor getParent();
    void addChild(PActor child);
    void removeChild(PActor child);
    Array<PActor> getChildren();

    Component::Transform& getTransform();

  protected:
    // Component::Transform& getTransform();
    void setParent(PActor parent);
    PActor parent;
    Array<PActor> children;
};
DEFINE_REF(Actor)
} // namespace Seele