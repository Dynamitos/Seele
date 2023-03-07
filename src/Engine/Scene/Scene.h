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

struct DirectionalLight
{
    Vector4 color;
    Vector4 direction;
};

struct PointLight
{
    Vector4 positionWS;
    //Vector4 positionVS;
    Vector4 colorRange;
};

#define MAX_DIRECTIONAL_LIGHTS 4
#define MAX_POINT_LIGHTS 256
struct LightEnv
{
    Gfx::PStructuredBuffer directionalLights;
    Gfx::PUniformBuffer numDirectional;
    Gfx::PStructuredBuffer pointLights;
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
    template<typename Component, typename Func>
    void view(Func func) requires std::is_invocable_v<Func, Component&>
    {
        registry.view<Component>().each(func);
    }
    Array<MeshBatch> getStaticMeshes();
    LightEnv getLightBuffer();
    Component::Skybox getSkybox();
    Gfx::PStructuredBuffer getSceneDataBuffer() const { return sceneDataBuffer;  }
    Gfx::PGraphics getGraphics() const { return graphics; }
    entt::registry registry;
private:
    struct PrimitiveSceneData
    {
        Matrix4 localToWorld;
        Matrix4 worldToLocal;
        Vector4 actorLocation;
    };
    Array<PrimitiveSceneData> sceneData;
    Gfx::PStructuredBuffer sceneDataBuffer;
    LightEnv lightEnv;
    PhysicsSystem physics;
    Gfx::PGraphics graphics;
};
DEFINE_REF(Scene)
} // namespace Seele