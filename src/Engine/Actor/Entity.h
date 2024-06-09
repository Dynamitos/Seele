#pragma once
#include "MinimalEngine.h"
#include "Scene/Scene.h"
#include <entt/entt.hpp>

namespace Seele {
// An entity describes a part of a scene
// It is just a wrapper the ID of a registry
class Entity {
  public:
    Entity(PScene scene);
    virtual ~Entity();

    template <typename Component, typename... Args> Component& attachComponent(Args&&... args) {
        return scene->attachComponent<Component>(identifier, std::forward<Args>(args)...);
    }
    template <typename Component> Component& accessComponent() { return scene->accessComponent<Component>(identifier); }
    template <typename Component> const Component& accessComponent() const { return scene->accessComponent<Component>(identifier); }

  protected:
    PScene scene;
    entt::entity identifier;
};
DEFINE_REF(Entity)
} // namespace Seele