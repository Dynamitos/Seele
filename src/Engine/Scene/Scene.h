#pragma once
#include "MinimalEngine.h"
#include "Actor/Actor.h"
#include "Graphics/GraphicsResources.h"
#include "Components/PrimitiveComponent.h"
#include "Graphics/MeshBatch.h"
#include "Material/Material.h"

namespace Seele
{
DECLARE_REF(Material);

class Scene
{
public:
    Scene();
    ~Scene();
    void tick(float deltaTime);
    void addActor(PActor actor);
    void addPrimitiveComponent(PPrimitiveComponent comp);

private:
    Map<PMaterial, MeshBatch> meshBatches;
    Array<PActor> rootActors;
    Array<PPrimitiveComponent> primitives;
    const static int constant = 10;
public:
    Map<PMaterial, MeshBatch> getMeshBatches();
};
} // namespace Seele