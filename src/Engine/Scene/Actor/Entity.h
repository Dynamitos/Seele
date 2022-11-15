#pragma once
#include <entt/entt.hpp>
#include "MinimalEngine.h"
#include "Scene/Scene.h"
namespace Seele
{
// An entity describes a part of a scene
// It is just a wrapper the ID of a registry
class Entity
{
public:
    Entity(PScene scene);
    virtual ~Entity();

    template<typename Component, typename... Args>
    void attachComponent(Args... args)
    {
        scene->attachComponent<Component>(identifier, args...);
    }
protected:
    PScene scene;
    entt::entity identifier;
};
DEFINE_REF(Entity)
} // namespace Seele