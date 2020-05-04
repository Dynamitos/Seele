#pragma once
#include "Component.h"
#include "Graphics/GraphicsResources.h"
#include "Material/MaterialInstance.h"

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
    PMaterialInstance instance;
    Gfx::PVertexBuffer vertexBuffer;
    Gfx::PIndexBuffer indexBuffer;
    friend class Scene;
};
DEFINE_REF(PrimitiveComponent);
} // namespace Seele