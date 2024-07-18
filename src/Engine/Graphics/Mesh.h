#pragma once
#include "Asset/MaterialInstanceAsset.h"
#include "Graphics/Buffer.h"
#include "VertexData.h"
#include "Graphics/RayTracing.h"

namespace Seele {
class Mesh {
  public:
    Mesh();
    ~Mesh();

    // transform from importing
    Matrix4 transform = Matrix4(1);
    VertexData* vertexData;
    MeshId id;
    uint64 vertexCount;
    PMaterialInstanceAsset referencedMaterial;
    Gfx::OBottomLevelAS blas;
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

  private:
};
DEFINE_REF(Mesh)
namespace Serialization {
template <> void save(ArchiveBuffer& buffer, const OMesh& ptr) { ptr->save(buffer); }
template <> void load(ArchiveBuffer& buffer, OMesh& ptr) {
    ptr = new Mesh();
    ptr->load(buffer);
}
} // namespace Serialization
} // namespace Seele