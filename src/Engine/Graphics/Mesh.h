#pragma once
#include "GraphicsResources.h"
#include "Asset/MaterialAsset.h"

namespace Seele
{
DECLARE_REF(VertexShaderInput)
DECLARE_NAME_REF(Gfx, IndexBuffer)
class Mesh
{
public:
    Mesh(PVertexShaderInput vertexInput, Gfx::PIndexBuffer indexBuffer);
    ~Mesh();

    Gfx::PIndexBuffer indexBuffer;
    PVertexShaderInput vertexInput;
    PMaterialAsset referencedMaterial;
private:
};
DEFINE_REF(Mesh)
} // namespace Seele