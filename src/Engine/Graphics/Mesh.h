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
    uint64 vertexCount;
    PMaterialInstance referencedMaterial;
    Array<Meshlet> meshlets;
    void save(ArchiveBuffer& buffer);
    void load(ArchiveBuffer& buffer);
private:
};
DEFINE_REF(Mesh)
} // namespace Seele