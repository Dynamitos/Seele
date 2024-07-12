#pragma once
#include "Component/Transform.h"
#include "Containers/List.h"
#include "Graphics/Buffer.h"
#include "Graphics/Command.h"
#include "Graphics/Descriptor.h"
#include "MeshData.h"
#include "Meshlet.h"
#include <entt/entt.hpp>

constexpr uint64 MAX_TEXCOORDS = 8;

namespace Seele {
DECLARE_REF(MaterialInstance)
DECLARE_REF(Mesh)
struct MeshId {
    uint64 id;
    operator uint64() const { return id; }
    std::strong_ordering operator<=>(const MeshId& other) const { return id <=> other.id; }
    bool operator==(const MeshId& other) const { return id == other.id; }
};
class VertexData {
  public:
    struct DrawCallOffsets {
        // offset into the instanceData and meshData arrays
        uint32 instanceOffset = 0;
        uint32 textureOffset = 0;
        uint32 samplerOffset = 0;
        uint32 floatOffset = 0;
    };

    struct MeshletCullingInfo {
        uint32_t cull;
    };
    struct BatchedDrawCall {
        PMaterialInstance materialInstance;
        DrawCallOffsets offsets;
        Array<InstanceData> instanceData;
        Array<MeshData> instanceMeshData;
        Array<Gfx::PBottomLevelAS> rayTracingData;
        Array<uint32> cullingOffsets;
    };
    struct MaterialData {
        PMaterial material;
        Array<BatchedDrawCall> instances;
    };
    struct TransparentDraw {
        PMaterialInstance matInst;
        VertexData* vertexData;
        DrawCallOffsets offsets;
        Vector worldPosition;
    };
    void resetMeshData();
    void updateMesh(entt::entity id, uint32 meshIndex, PMesh mesh, Component::Transform& transform);
    void createDescriptors();
    void loadMesh(MeshId id, Array<uint32> indices, Array<Meshlet> meshlets);
    void commitMeshes();
    MeshId allocateVertexData(uint64 numVertices);
    uint64 getMeshOffset(MeshId id) const { return meshOffsets[id]; }
    uint64 getMeshVertexCount(MeshId id) { return meshVertexCounts[id]; }
    virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) = 0;
    virtual void deserializeMesh(MeshId id, ArchiveBuffer& buffer) = 0;
    virtual void bindBuffers(Gfx::PRenderCommand command) = 0;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() = 0;
    virtual Gfx::PDescriptorSet getVertexDataSet() = 0;
    virtual std::string getTypeName() const = 0;
    virtual Gfx::PShaderBuffer getPositionBuffer() const = 0;
    Gfx::PIndexBuffer getIndexBuffer() const { return indexBuffer; }
    uint32* getIndexData() const { return indices.data(); }
    Gfx::PDescriptorLayout getInstanceDataLayout() { return instanceDataLayout; }
    Gfx::PDescriptorSet getInstanceDataSet() { return descriptorSet; }
    const Array<MaterialData>& getMaterialData() const { return materialData; }
    const Array<TransparentDraw>& getTransparentData() const { return transparentData; }
    const Array<Gfx::PBottomLevelAS>& getRayTracingData() const { return rayTracingScene; }
    const MeshData& getMeshData(MeshId id) const { return meshData[id]; }
    void registerBottomLevelAccelerationStructure(Gfx::PBottomLevelAS blas) { dataToBuild.add(blas); }
    uint64 getIndicesOffset(uint32 meshletIndex) { return meshlets[meshletIndex].indicesOffset; }
    uint64 getNumInstances() const { return instanceData.size(); }
    static List<VertexData*> getList();
    static VertexData* findByTypeName(std::string name);
    virtual void init(Gfx::PGraphics graphics);
    virtual void destroy();

    struct CullingMapping {
        uint64 instanceId;
        uint32 cullingOffset;
    };
    static CullingMapping getCullingMapping(entt::entity id, uint32 meshIndex, uint32 numMeshlets);
    static uint64 getInstanceCount() { return instanceCount; }
    static uint64 getMeshletCount() { return meshletCount; }

  protected:
    virtual void resizeBuffers() = 0;
    virtual void updateBuffers() = 0;
    VertexData();
    struct MeshletDescription {
        AABB bounding;
        // number of relevant entries in the vertexIndices array
        uint32 vertexCount;
        // number of relevant entries in the primitiveIndices array
        uint32 primitiveCount;
        // starting offset into the vertexIndices array
        uint32 vertexOffset;
        // starting offset into the primitiveIndices array
        uint32 primitiveOffset;
        Vector color;
        // gets added to vertex indices so that they reference the global mesh bool
        uint32 indicesOffset = 0;
    };
    std::mutex materialDataLock;
    Array<MaterialData> materialData;
    Array<TransparentDraw> transparentData;

    std::mutex vertexDataLock;
    Array<MeshData> meshData;
    Array<uint64> meshOffsets;
    Array<uint64> meshVertexCounts;

    Array<MeshletDescription> meshlets;
    Array<uint8> primitiveIndices;
    Array<uint32> vertexIndices;
    Array<uint32> indices;

    struct MeshMapping {
        entt::entity id;
        uint32 meshId;
        auto operator<=>(const MeshMapping& other) const = default;
    };
    static Map<MeshMapping, CullingMapping> instanceIdMap;
    static uint64 instanceCount;
    static uint64 meshletCount;

    Gfx::PGraphics graphics;
    Gfx::ODescriptorLayout instanceDataLayout;
    // for mesh shading
    Gfx::OShaderBuffer meshletBuffer;
    Gfx::OShaderBuffer vertexIndicesBuffer;
    Gfx::OShaderBuffer primitiveIndicesBuffer;
    Gfx::OShaderBuffer cullingOffsetBuffer;
    // for legacy pipeline
    Gfx::OIndexBuffer indexBuffer;
    Array<Gfx::PBottomLevelAS> dataToBuild;
    // Material data
    Array<InstanceData> instanceData;
    Gfx::OShaderBuffer instanceBuffer;

    Array<MeshData> instanceMeshData;
    Gfx::OShaderBuffer instanceMeshDataBuffer;

    Array<Gfx::PBottomLevelAS> rayTracingScene;
    
    Array<InstanceData> transparentInstanceData;
    Gfx::OShaderBuffer transparentInstanceDataBuffer;
    Array<MeshData> transparentMeshData;
    Gfx::OShaderBuffer transparentMeshDataBuffer;

    Gfx::PDescriptorSet descriptorSet;
    uint64 idCounter;
    uint64 head;
    uint64 verticesAllocated;
    bool dirty;
};
} // namespace Seele
