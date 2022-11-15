#pragma once
#include <entt/entt.hpp>
#include "MinimalEngine.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/MeshBatch.h"

namespace Seele
{
DECLARE_REF(Material)
DECLARE_REF(Entity)

struct DirectionalLight
{
    Math::Vector4 color;
    Math::Vector4 direction;
    Math::Vector4 intensity;
};

struct PointLight
{
    Math::Vector4 positionWS;
    //Vector4 positionVS;
    Math::Vector4 colorRange;
};

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 256
struct LightEnv
{
    DirectionalLight directionalLights[MAX_DIRECTIONAL_LIGHTS];
    PointLight pointLights[MAX_POINT_LIGHTS];
    uint32 numDirectionalLights;
    uint32 numPointLights;
};

class Scene
{
public:
    Scene(Gfx::PGraphics graphics);
    ~Scene();
    void start();
    void beginUpdate(double deltaTime);
    void commitUpdate();
    entt::entity createEntity()
    {
        return registry.create();
    }
    template<typename Component, typename... Args>
    Component& attachComponent(entt::entity entity, Args... args)
    {
        return registry.emplace<Component>(entity, args...);
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
    Array<StaticMeshBatch> getStaticMeshes();
    LightEnv getLightBuffer() const;
    
    entt::registry registry;
private:
    Gfx::PGraphics graphics;
};
DEFINE_REF(Scene)
} // namespace Seele