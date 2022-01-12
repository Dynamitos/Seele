#pragma once
#include "Component.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Mesh.h"
#include "Material/MaterialAsset.h"
#include "Graphics/MeshBatch.h"

namespace Seele
{
DECLARE_REF(MeshAsset)
class PrimitiveComponent : public Component
{
public:
    PrimitiveComponent();
    PrimitiveComponent(PMeshAsset asset);
    ~PrimitiveComponent();
    virtual void notifySceneAttach(PScene scene) override;
    Matrix4 getRenderMatrix();
    std::vector<StaticMeshBatch>& getStaticMeshes()
    {
        return staticMeshes;
    }
private:
    std::vector<PMaterialAsset> materials;
    Gfx::PUniformBuffer uniformBuffer;
    std::vector<StaticMeshBatch> staticMeshes;
    friend class Scene;
};
DEFINE_REF(PrimitiveComponent)
} // namespace Seele