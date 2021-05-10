#pragma once
#include "MinimalEngine.h"
#include "Actor/Actor.h"
#include "Graphics/GraphicsResources.h"
#include "Components/PrimitiveComponent.h"
#include "Graphics/MeshBatch.h"
#include "Material/Material.h"
#include "SceneUpdater.h"

namespace Seele
{
DECLARE_REF(Material)

struct DirectionalLight
{
    Vector4 color;
    Vector4 direction;
    Vector4 intensity;
};

struct PointLight
{
    Vector4 positionWS;
    Vector4 positionVS;
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
    void tick(double deltaTime);
    void addActor(PActor actor);
    void addPrimitiveComponent(PPrimitiveComponent comp);

    const Array<PPrimitiveComponent>& getPrimitives() const { return primitives; }
    const Array<MeshBatch>& getStaticMeshes() const { return staticMeshes; }
    const Gfx::PStructuredBuffer& getLightBuffer() const { return lightBuffer; }
    UPSceneUpdater& getSceneUpdater() { return updater; }
private:
    Array<MeshBatch> staticMeshes;
    Array<PActor> rootActors;
    Array<PPrimitiveComponent> primitives;
    LightEnv lightEnv;
    Gfx::PStructuredBuffer lightBuffer;
    Gfx::PGraphics graphics;
    UPSceneUpdater updater;
};
} // namespace Seele