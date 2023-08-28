#pragma once
#include "GraphicsResources.h"
#include "Asset/MaterialAsset.h"

namespace Seele
{
DECLARE_REF(MeshletBuffer)
DECLARE_NAME_REF(Gfx, IndexBuffer)
class Mesh
{
public:
    Mesh(PMeshletBuffer meshlets);
    ~Mesh();

    PMeshletBuffer meshlets;
    PVertexData vertexData;
    PMaterialAsset referencedMaterial;
private:
};
DEFINE_REF(Mesh)
} // namespace Seele