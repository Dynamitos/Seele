#pragma once
#include "GraphicsResources.h"
#include "Asset/MaterialAsset.h"

namespace Seele
{
DECLARE_REF(TopologyData)
DECLARE_REF(VertexData)
class Mesh
{
public:
    Mesh();
    ~Mesh();

    PTopologyData meshlets;
    PVertexData vertexData;
    PMaterialAsset referencedMaterial;
private:
};
DEFINE_REF(Mesh)
} // namespace Seele