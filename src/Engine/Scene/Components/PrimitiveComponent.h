#pragma once
#include "Component.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Mesh.h"
#include "Material/MaterialInstance.h"
#include "Graphics/MeshBatch.h"

namespace Seele
{
class PrimitiveComponent : public Component
{
public:
    PrimitiveComponent();
    ~PrimitiveComponent();
    virtual void notifySceneAttach(PScene scene) override;
    Matrix4 getRenderMatrix();

private:
    Array<PMaterialInstance> materials;
    Gfx::PUniformBuffer uniformBuffer;
    Array<StaticMeshBatch> staticMeshes;
    PMesh mesh;
    friend class Scene;
};
DEFINE_REF(PrimitiveComponent);
} // namespace Seele