#pragma once
#include "Entity.h"
#include "Component/Transform.h"

namespace Seele
{
DECLARE_REF(Actor)
// Actors are entities that are part of the scene hierarchy
// In order for that hierarchy to make sense it requires at least a transform component
class Actor : public Entity
{
public:
    Actor(PScene scene);
    virtual ~Actor();

    PActor getParent();
    void addChild(PActor child);
    void removeChild(PActor child);
    Array<PActor> getChildren();
    
    // Returns a read-only copy of the actors transform
    const Component::Transform& getTransform() const;

protected:
    //Component::Transform& getTransform();
    void setParent(PActor parent);
    PActor parent;
    Array<PActor> children;
};
DEFINE_REF(Actor)
} // namespace Seele