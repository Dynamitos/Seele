#pragma once
#include "Graphics/Initializer.h"
#include "Material/MaterialInstance.h"
#include "Component/Transform.h"
#include "Containers/List.h"
#include "Graphics/Resources.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Buffer.h"

namespace Seele
{
DECLARE_REF(Mesh)
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
    StaticArray<uint32, Gfx::numVerticesPerMeshlet> uniqueVertices; // unique vertiex indices in the vertex data
    StaticArray<uint8, Gfx::numPrimitivesPerMeshlet * 3> primitiveLayout; // indices into the uniqueVertices array, only uint8 needed
    uint32 numVertices;
    uint32 numPrimitives;
    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);
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
        Gfx::PIndexBuffer indexBuffer;
    };
    struct MaterialInstanceData
    {
        PMaterialInstance materialInstance;
        Gfx::PDescriptorSet descriptorSet;
        uint32 numMeshes; // not necessarily equal to meshes.size() if a MeshId has multiple meshes
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
    void updateMesh(const Component::Transform& transform, PMesh mesh);
    void loadMesh(MeshId id, Array<Meshlet> meshlets);
    void createDescriptors();
    MeshId allocateVertexData(uint64 numVertices);
    uint32 getMeshOffset(MeshId id);
    virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) = 0;
    virtual void deserializeMesh(MeshId id, ArchiveBuffer& buffer) = 0;
    virtual void bindBuffers(Gfx::PRenderCommand command) = 0;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() = 0;
    virtual Gfx::PDescriptorSet getVertexDataSet() = 0;
    virtual std::string getTypeName() const = 0;
    Gfx::PDescriptorLayout getInstanceDataLayout() { return instanceDataLayout; }
    const Map<std::string, MaterialData>& getMaterialData() const { return materialData; }
    const Array<MeshData>& getMeshData(MeshId id) { return meshData[id]; }
    static List<VertexData*> getList();
    static VertexData* findByTypeName(std::string name);
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
    Map<MeshId, uint64> meshOffsets;
    Array<MeshletDescription> meshlets;
    Array<uint8> primitiveIndices;
    Array<uint32> vertexIndices;
    Gfx::PGraphics graphics;
    Gfx::ODescriptorLayout instanceDataLayout;
    // for mesh shading
    Gfx::OShaderBuffer meshletBuffer;
    Gfx::OShaderBuffer vertexIndicesBuffer;
    Gfx::OShaderBuffer primitiveIndicesBuffer;
    // for legacy pipeline
    Gfx::OIndexBuffer indexBuffer;
    uint64 idCounter;
	uint64 head;
	uint64 verticesAllocated;
    bool dirty;
};
}
