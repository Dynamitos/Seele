#pragma once
#include "Asset/MaterialInstanceAsset.h"
#include "VertexData.h"
#include "Graphics/Buffer.h"

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
    Gfx::OIndexBuffer indexBuffer;
    PMaterialInstanceAsset referencedMaterial;
    Array<uint32> indices;
    Array<Meshlet> meshlets;
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
private:
};
DEFINE_REF(Mesh)
namespace Serialization
{
    template<>
    void save(ArchiveBuffer& buffer, const OMesh& ptr)
    {
        ptr->save(buffer);
    }
    template<>
    void load(ArchiveBuffer& buffer, OMesh& ptr)
    {
        ptr = new Mesh();
        ptr->load(buffer);
    }
} // namespace Serialization
} // namespace Seele