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
    std::strong_ordering operator<=>(const MeshId& other) const
    {
        return id <=> other.id;
    }
};
struct Meshlet
{
    uint32 uniqueVertices[Gfx::numVerticesPerMeshlet]; // unique vertiex indices in the vertex data
    uint8 primitiveLayout[Gfx::numPrimitivesPerMeshlet * 3]; // indices into the uniqueVertices array, only uint8 needed
    uint32 numVertices;
    uint32 numPrimitives;
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
        uint32 indicesOffset;
    };
    void resetMeshData();
    void updateMesh(const Component::Transform& transform, const Component::Mesh& mesh);
    void loadMesh(MeshId id, Array<Meshlet> meshlets);
    void createDescriptors();
    MeshId allocateVertexData(uint64 numVertices);
    virtual void bindBuffers(Gfx::PRenderCommand command) = 0;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() = 0;
    virtual Gfx::PDescriptorSet getVertexDataSet() = 0;
    virtual std::string getTypeName() const = 0;
    Gfx::PDescriptorLayout getInstanceDataLayout() { return instanceDataLayout; }
    const Map<std::string, MaterialData>& getMaterialData() const { return materialData; }
    const Array<MeshData>& getMeshData(MeshId id) { return meshData[id]; }
    static List<VertexData*> getList();
    virtual void init(Gfx::PGraphics graphics);
protected:
    virtual void resizeBuffers() = 0;
    virtual void updateBuffers() = 0;
    VertexData();
    struct MeshletAABB
    {
        Vector min;
        float pad0;
        Vector max;
        float pad1;
    };
    struct MeshletDescription
    {
        MeshletAABB boundingBox;
        uint32_t vertexCount;
        uint32_t primitiveCount;
        uint32_t vertexOffset;
        uint32_t primitiveOffset;
    };
    Map<std::string, MaterialData> materialData;
    Map<MeshId, Array<MeshData>> meshData;
    Map<MeshId, uint64_t> meshOffsets;
    Array<MeshletDescription> meshlets;
    Array<uint8> primitiveIndices;
    Array<uint32> vertexIndices;
    Gfx::PGraphics graphics;
    Gfx::PDescriptorLayout instanceDataLayout;
    // for mesh shading
    Gfx::PShaderBuffer meshletBuffer;
    Gfx::PShaderBuffer vertexIndicesBuffer;
    Gfx::PShaderBuffer primitiveIndicesBuffer;
    // for legacy pipeline
    Gfx::PIndexBuffer indexBuffer;
    uint64 idCounter;
	uint64 head;
	uint64 verticesAllocated;
    bool dirty;
};
}
