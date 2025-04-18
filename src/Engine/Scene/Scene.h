#pragma once
#include "MinimalEngine.h"
#include "Component/Skybox.h"
#include "Graphics/Graphics.h"
#include "LightEnvironment.h"
#include "Physics/PhysicsSystem.h"
#include <entt/entt.hpp>
#include <iostream>


namespace Seele {
DECLARE_REF(Material)
DECLARE_REF(Entity)
class Scene {
  public:
    Scene(Gfx::PGraphics graphics);
    ~Scene();
    void bakeLighting();
    void update(float deltaTime);
    entt::entity createEntity() { return registry.create(); }
    void destroyEntity(entt::entity identifier) { registry.destroy(identifier); }
    template <typename Component, typename... Args> Component& attachComponent(entt::entity entity, Args&&... args) {
        return registry.emplace<Component>(entity, std::forward<Args>(args)...);
    }
    template <typename Component> Component& accessComponent(entt::entity entity) { return registry.get<Component>(entity); }
    template <typename Component> const Component& accessComponent(entt::entity entity) const { return registry.get<Component>(entity); }
    template <typename... Component, typename Func>
    void view(Func func)
        requires std::is_invocable_v<Func, Component&...> || std::is_invocable_v<Func, entt::entity, Component&...>
    {
        registry.view<Component...>().each(func);
    }
    template <typename Component> auto constructCallback() { return registry.on_construct<Component>(); }
    template <typename Component> auto destroyCallback() { return registry.on_destroy<Component>(); }
    PLightEnvironment getLightEnvironment() { return lightEnv; }
    Gfx::PGraphics getGraphics() const { return graphics; }
    entt::registry registry;

  private:
    Gfx::PGraphics graphics;
    OLightEnvironment lightEnv;
    PhysicsSystem physics;
    Array<Gfx::OTexture2D> lightMaps;
};
DEFINE_REF(Scene)
} // namespace Seele