#pragma once
#include "Asset/MaterialInstanceAsset.h"
#include "VertexData.h"

namespace Seele
{
class Mesh
{
public:
    Mesh();
    ~Mesh();

    VertexData* vertexData;
    MeshId id;
    PMaterialInstanceAsset referencedMaterial;
private:
};
DEFINE_REF(Mesh)
} // namespace Seele