#pragma once
#include "Material/MaterialInstance.h"
#include "Component/Transform.h"
#include "Containers/List.h"
#include "Graphics/Resources.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Buffer.h"

namespace Seele
{
namespace Component
{
    struct Mesh;
}
struct MeshId
{
    uint64 id;
};
class VertexData
{
public:
    struct InstanceData
    {
        Matrix4 transformMatrix;
    };
    struct MeshInstanceData
    {
        MeshId id;
        InstanceData instance;
    };
    struct MaterialInstanceData
    {
        PMaterialInstance materialInstance;
        Gfx::PDescriptorSet descriptorSet;
        uint32_t numMeshes; // not necessarily equal to meshes.size() if a MeshId has multiple meshes
        Array<MeshInstanceData> meshes;
    };
    struct MaterialData
    {
        PMaterial material;
        Map<uint64, MaterialInstanceData> instances;
    };
    struct MeshData
    {
        uint32 numMeshlets;
        uint32 meshletOffset;
    };
    void resetMeshData();
    void updateMesh(const Component::Transform& transform, const Component::Mesh& mesh);
    void createDescriptors();
    virtual MeshId allocateVertexData(uint64 numVertices) = 0;
    virtual void bindBuffers(Gfx::PRenderCommand command) = 0;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() = 0;
    virtual Gfx::PDescriptorSet getVertexDataSet() = 0;
    virtual Gfx::PDescriptorLayout getInstanceDataLayout() = 0;
    virtual std::string getTypeName() const = 0;
    const Map<std::string, MaterialData>& getMaterialData() const { return materialData; }
    const Array<MeshData>& getMeshData(MeshId id) { return meshData[id]; }
    static List<VertexData*> getList();
protected:
    struct MeshletAABB
    {
        Vector min;
        float pad0;
        Vector max;
        float pad1;
    };
    struct MeshletData
    {
        MeshletAABB boundingBox;
        uint32_t vertexCount;
        uint32_t primiticeCount;
        uint32_t vertexOffset;
        uint32_t primitiveOffset;
    };
    Map<std::string, MaterialData> materialData;
    Map<MeshId, Array<MeshData>> meshData;
    Array<MeshletData> meshlets;
    Gfx::PDescriptorLayout instanceDataLayout;
    Gfx::PGraphics graphics;
    // for mesh shading
    Gfx::PShaderBuffer meshletBuffer;
    // for legacy pipeline
    Gfx::PIndexBuffer indexBuffer;
    VertexData(Gfx::PGraphics graphics);
    uint64 idCounter;
};
}
