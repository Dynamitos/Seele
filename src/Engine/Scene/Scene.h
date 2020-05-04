#pragma once
#include "MinimalEngine.h"
#include "Actor/Actor.h"
#include "Graphics/GraphicsResources.h"
#include "Components/PrimitiveComponent.h"

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
    Map<PMaterial, DrawState> meshBatches;
    Array<PActor> rootActors;
    Array<PPrimitiveComponent> primitives;

public:
    Map<PMaterial, DrawState> getMeshBatches();
};
} // namespace Seele