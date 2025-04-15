#pragma once
#include "Component/Transform.h"
#include "Containers/List.h"
#include "Graphics/Buffer.h"
#include "Graphics/Command.h"
#include "Graphics/Descriptor.h"
#include "MeshData.h"
#include "Meshlet.h"
#include <entt/entt.hpp>

constexpr uint32 MAX_TEXCOORDS = 8;

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
        InstanceData instanceData;
        MeshData meshData;
        uint32 cullingOffset;
        Gfx::PBottomLevelAS rayTracingScene;
    };
    void resetMeshData();
    void updateMesh(uint32 meshletOffset, PMesh mesh, Component::Transform& transform);
    virtual void createDescriptors();
    void loadMesh(MeshId id, Array<uint32> indices, Array<Meshlet> meshlets);
    virtual void removeMesh(MeshId id);
    void commitMeshes();
    MeshId allocateVertexData(uint64 numVertices);
    uint64 getMeshOffset(MeshId id) const { return registeredMeshes[id].vertexOffset; }
    uint64 getMeshVertexCount(MeshId id) { return registeredMeshes[id].vertexCount; }
    virtual void serializeMesh(MeshId id, ArchiveBuffer& buffer);
    virtual uint64 deserializeMesh(MeshId id, ArchiveBuffer& buffer);
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
    const Array<MeshData>& getMeshData(MeshId id) const { return registeredMeshes[id].meshData; }
    void registerBottomLevelAccelerationStructure(Gfx::PBottomLevelAS blas) { dataToBuild.add(blas); }
    uint32 getIndicesOffset(uint32 meshletIndex) { return meshlets[meshletIndex].indicesOffset; }
    uint64 getNumInstances() const { return instanceData.size(); }
    static void addVertexDataInstance(VertexData* vertexData);
    static List<VertexData*> getList();
    static VertexData* findByTypeName(std::string name);
    virtual void init(Gfx::PGraphics graphics);
    virtual void destroy();

    uint32 addCullingMapping(MeshId id);
    static uint64 getMeshletCount() { return meshletCount; }
    constexpr static const char* CULLINGDATA_NAME = "cullingData";

  protected:
    virtual void resizeBuffers() = 0;
    virtual void updateBuffers() = 0;
    VertexData();
    struct MeshletDescription {
        AABB bounding;
        // range into vertexIndices array
        PoolRange vertexIndices;
        // range into primitiveIndices array
        PoolRange primitiveIndices;
        Vector color;
        // gets added to vertex indices so that they reference the global mesh pool
        uint32 indicesOffset = 0;
    };
    std::mutex materialDataLock;
    Array<MaterialData> materialData;
    Array<TransparentDraw> transparentData;

    std::mutex vertexDataLock;
    struct RegisteredMesh
    {
        // each mesh id can have multiple meshdata, in case it needs to be split for having too many meshlets
        Array<MeshData> meshData;
        uint64 vertexOffset;
        uint64 vertexCount;
    };
    Array<RegisteredMesh> registeredMeshes;

    Array<MeshletDescription> meshlets;
    Array<uint8> primitiveIndices;
    Array<uint32> vertexIndices;
    Array<uint32> indices;

    static uint64 meshletCount;

    Gfx::PGraphics graphics;
    Gfx::ODescriptorLayout instanceDataLayout;
    // for mesh shading
    Gfx::OShaderBuffer meshletBuffer;
    constexpr static const char* MESHLET_NAME = "meshlets";
    Gfx::OShaderBuffer vertexIndicesBuffer;
    constexpr static const char* VERTEXINDICES_NAME = "vertexIndices";
    Gfx::OShaderBuffer primitiveIndicesBuffer;
    constexpr static const char* PRIMITIVEINDICES_NAME = "primitiveIndices";
    Gfx::OShaderBuffer cullingOffsetBuffer;
    constexpr static const char* CULLINGOFFSETS_NAME = "cullingOffsets";

    // for legacy pipeline
    Gfx::OIndexBuffer indexBuffer;
    constexpr static const char* INDEXBUFFER_NAME = "indexBuffer";
    Array<Gfx::PBottomLevelAS> dataToBuild;
    // Material data
    Array<InstanceData> instanceData;
    Gfx::OShaderBuffer instanceBuffer;
    constexpr static const char* INSTANCES_NAME = "instances";

    Array<MeshData> instanceMeshData;
    Gfx::OShaderBuffer instanceMeshDataBuffer;
    constexpr static const char* MESHDATA_NAME = "meshData";

    Array<Gfx::PBottomLevelAS> rayTracingScene;

    Gfx::PDescriptorSet descriptorSet;
    uint64 idCounter;
    uint64 head;
    uint64 verticesAllocated;
    bool dirty;
};
} // namespace Seele
