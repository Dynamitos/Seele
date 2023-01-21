#pragma once
#include "GraphicsResources.h"
#include "Material/MaterialInterface.h"

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
    PMaterialInterface referencedMaterial;
private:
};
DEFINE_REF(Mesh)
} // namespace Seele