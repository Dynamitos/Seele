#pragma once
#include "Graphics/Initializer.h"
#include "Material/MaterialInstance.h"
#include "Component/Transform.h"
#include "Containers/List.h"
#include "Graphics/Command.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Buffer.h"
#include "Meshlet.h"

constexpr uint64 MAX_TEXCOORDS = 8;

namespace Seele
{
  DECLARE_REF(Mesh)
  struct MeshId
  {
    uint64 id;
    std::strong_ordering operator<=>(const MeshId &other) const
    {
      return id <=> other.id;
    }
    bool operator==(const MeshId &other) const
    {
      return id == other.id;
    }
  };
  class VertexData
  {
  public:
    struct InstanceData
    {
      Matrix4 transformMatrix;
      Matrix4 inverseTransformMatrix;
    };
    struct MeshData
    {
      AABB bounding;
      uint32 numMeshlets = 0;
      uint32 meshletOffset = 0;
      uint32 firstIndex = 0;
      uint32 numIndices = 0;
    };
    struct DrawCallOffsets
    {
      uint32 instanceOffset = 0;
    };

    struct MeshletCullingInfo
    {
      uint64_t cull[256 / 64];
    };
    struct BatchedDrawCall
    {
      PMaterialInstance materialInstance;
      DrawCallOffsets offsets;
      Array<InstanceData> instanceData;
      Array<MeshData> instanceMeshData;
    };
    struct MaterialData
    {
      PMaterial material;
      Array<BatchedDrawCall> instances;
    };
    void resetMeshData();
    void updateMesh(PMesh mesh, Component::Transform &transform);
    void createDescriptors();
    void loadMesh(MeshId id, Array<uint32> indices, Array<Meshlet> meshlets);
    MeshId allocateVertexData(uint64 numVertices);
    uint64 getMeshOffset(MeshId id);
    uint64 getMeshVertexCount(MeshId id);
    virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer &buffer) = 0;
    virtual void deserializeMesh(MeshId id, ArchiveBuffer &buffer) = 0;
    virtual void bindBuffers(Gfx::PRenderCommand command) = 0;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() = 0;
    virtual Gfx::PDescriptorSet getVertexDataSet() = 0;
    virtual std::string getTypeName() const = 0;
    Gfx::PIndexBuffer getIndexBuffer() { return indexBuffer; }
    Gfx::PDescriptorLayout getInstanceDataLayout() { return instanceDataLayout; }
    Gfx::PDescriptorSet getInstanceDataSet() { return descriptorSet; }
    const Array<MaterialData> &getMaterialData() const { return materialData; }
    const MeshData &getMeshData(MeshId id) { return meshData[id]; }
    uint64 getIndicesOffset(uint32 meshletIndex) { return meshlets[meshletIndex].indicesOffset; }
    uint64 getNumInstances() const { return instanceData.size(); }
    static List<VertexData *> getList();
    static VertexData *findByTypeName(std::string name);
    virtual void init(Gfx::PGraphics graphics);
    virtual void destroy();

  protected:
    virtual void resizeBuffers() = 0;
    virtual void updateBuffers() = 0;
    VertexData();
    struct MeshletDescription
    {
      AABB bounding;
      uint32 vertexCount;
      uint32 primitiveCount;
      uint32 vertexOffset;
      uint32 primitiveOffset;
      Vector color;
      uint32 indicesOffset = 0;
    };
    std::mutex materialDataLock;
    Array<MaterialData> materialData;
    std::mutex vertexDataLock;
    Map<MeshId, MeshData> meshData;
    Map<MeshId, uint64> meshOffsets;
    Map<MeshId, uint64> meshVertexCounts;
    Array<MeshletDescription> meshlets;
    Array<uint8> primitiveIndices;
    Array<uint32> vertexIndices;
    Array<uint32> indices;
    Gfx::PGraphics graphics;
    Gfx::ODescriptorLayout instanceDataLayout;
    // for mesh shading
    Gfx::OShaderBuffer meshletBuffer;
    Gfx::OShaderBuffer vertexIndicesBuffer;
    Gfx::OShaderBuffer primitiveIndicesBuffer;
    // Holds culling information for every meshlet for each instance
    Gfx::OShaderBuffer cullingBuffer;
    Gfx::OShaderBuffer cullingOffsetBuffer;
    // for legacy pipeline
    Gfx::OIndexBuffer indexBuffer;
    // Material data
    Array<InstanceData> instanceData;
    Gfx::OShaderBuffer instanceBuffer;
    Array<MeshData> instanceMeshData;
    Gfx::OShaderBuffer instanceMeshDataBuffer;
    Gfx::PDescriptorSet descriptorSet;
    uint64 idCounter;
    uint64 head;
    uint64 verticesAllocated;
    bool dirty;
  };
}
