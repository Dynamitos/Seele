#pragma once
#include "MinimalEngine.h"
#include "Actor/Actor.h"
#include "Graphics/GraphicsResources.h"
#include "Components/PrimitiveComponent.h"
#include "Graphics/MeshBatch.h"

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
    void start();
    Job beginUpdate(double deltaTime);
    Job commitUpdate();
    void addActor(PActor actor);
    void addPrimitiveComponent(PPrimitiveComponent comp);

    const std::vector<PPrimitiveComponent>& getPrimitives() const { return primitives; }
    const std::vector<StaticMeshBatch>& getStaticMeshes() const { return staticMeshes; }
    LightEnv& getLightBuffer() { return lightEnv; }
private:
    std::vector<StaticMeshBatch> staticMeshes;
    std::vector<PActor> rootActors;
    std::vector<PPrimitiveComponent> primitives;
    LightEnv lightEnv;
    Gfx::PGraphics graphics;
};
} // namespace Seele