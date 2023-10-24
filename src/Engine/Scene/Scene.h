#pragma once
#include <entt/entt.hpp>
#include "MinimalEngine.h"
#include "Graphics/Graphics.h"
#include "Graphics/MeshBatch.h"
#include "Physics/PhysicsSystem.h"
#include "Component/Skybox.h"

namespace Seele
{
DECLARE_REF(Material)
DECLARE_REF(Entity)

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 256
struct LightEnv
{
    Gfx::PShaderBuffer directionalLights;
    Gfx::PUniformBuffer numDirectional;
    Gfx::PShaderBuffer pointLights;
    Gfx::PUniformBuffer numPoints;
};

class Scene
{
public:
    Scene(Gfx::PGraphics graphics);
    ~Scene();
    void update(float deltaTime);
    entt::entity createEntity()
    {
        return registry.create();
    }
    void destroyEntity(entt::entity identifier)
    {
        registry.destroy(identifier);
    }
    template<typename Component, typename... Args>
    Component& attachComponent(entt::entity entity, Args&&... args)
    {
        return registry.emplace<Component>(entity, std::forward<Args>(args)...);
    }
    template<typename Component>
    Component& accessComponent(entt::entity entity)
    {
        return registry.get<Component>(entity);
    }
    template<typename Component>
    const Component& accessComponent(entt::entity entity) const
    {
        return registry.get<Component>(entity);
    }
    template<typename... Component, typename Func>
    void view(Func func) requires std::is_invocable_v<Func, Component&...> || std::is_invocable_v<Func, entt::entity, Component&...>
    {
        registry.view<Component...>().each(func);
    }
    LightEnv getLightBuffer();
    Component::Skybox getSkybox();
    Gfx::PGraphics getGraphics() const { return graphics; }
    entt::registry registry;
private:
    LightEnv lightEnv;
    PhysicsSystem physics;
    Gfx::PGraphics graphics;
};
DEFINE_REF(Scene)
} // namespace Seele