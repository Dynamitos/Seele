#pragma once
#include <entt/entt.hpp>
#include "MinimalEngine.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/MeshBatch.h"
#include "Physics/PhysicsSystem.h"

namespace Seele
{
DECLARE_REF(Material)
DECLARE_REF(Entity)

struct DirectionalLight
{
    Vector4 color;
    Vector4 direction;
    Vector4 intensity;
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
    Array<StaticMeshBatch> getStaticMeshes();
    LightEnv getLightBuffer() const;
    Gfx::PStructuredBuffer getSceneDataBuffer() const { return sceneDataBuffer;  }
    
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
    PhysicsSystem physics;
    Gfx::PGraphics graphics;
};
DEFINE_REF(Scene)
} // namespace Seele