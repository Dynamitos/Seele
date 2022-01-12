#pragma once
#include "GraphicsResources.h"
#include "Material/MaterialAsset.h"

namespace Seele
{
DECLARE_REF(MaterialAsset)
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