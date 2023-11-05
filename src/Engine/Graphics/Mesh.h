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
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
private:
};
DEFINE_REF(Mesh)
namespace Serialization
{
    template<>
    static void save(ArchiveBuffer& buffer, const OMesh& ptr)
    {
        ptr->save(buffer);
    }
    template<>
    static void load(ArchiveBuffer& buffer, OMesh& ptr)
    {
        ptr = new Mesh();
        ptr->load(buffer);
    }
} // namespace Serialization
} // namespace Seele