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
    Scene(Gfx::PGraphics graphics);
    ~Scene();
    void tick(float deltaTime);
    void addActor(PActor actor);
    void addPrimitiveComponent(PPrimitiveComponent comp);

    const Array<PPrimitiveComponent>& getPrimitives() const { return primitives; }
    const Array<MeshBatch>& getStaticMeshes() const { return staticMeshes; }
private:
    Array<MeshBatch> staticMeshes;
    Array<PActor> rootActors;
    Array<PPrimitiveComponent> primitives;
    Gfx::PGraphics graphics;
};
} // namespace Seele