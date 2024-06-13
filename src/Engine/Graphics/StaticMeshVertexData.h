#pragma once
#include "Graphics/Command.h"
#include "Graphics/Initializer.h"
#include "Math/Vector.h"
#include "VertexData.h"
#include "entt/entt.hpp"


namespace Seele {
class StaticMeshVertexData : public VertexData {
  public:
    struct StaticMatInstance {
        PMaterialInstance instance;
        Array<uint32> meshletIds;
        Gfx::OShaderBuffer culledMeshletBuffer;
        // Gfx::OShaderBuffer indirectDrawBuffer;
    };
    struct StaticMatData {
        PMaterial material;
        Array<StaticMatInstance> staticInstance;
    };
    StaticMeshVertexData();
    virtual ~StaticMeshVertexData();
    static StaticMeshVertexData* getInstance();
    void loadPositions(MeshId id, const Array<Vector>& data);
    void loadTexCoords(MeshId id, uint64 index, const Array<Vector2>& data);
    void loadNormals(MeshId id, const Array<Vector>& data);
    void loadTangents(MeshId id, const Array<Vector>& data);
    void loadBiTangents(MeshId id, const Array<Vector>& data);
    void loadColors(MeshId id, const Array<Vector>& data);
    virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) override;
    virtual void deserializeMesh(MeshId id, ArchiveBuffer& buffer) override;
    virtual void init(Gfx::PGraphics graphics) override;
    virtual void destroy() override;
    virtual void bindBuffers(Gfx::PRenderCommand command) override;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() override;
    virtual Gfx::PDescriptorSet getVertexDataSet() override;
    virtual std::string getTypeName() const override { return "StaticMeshVertexData"; }
    virtual Gfx::PShaderBuffer getPositionBuffer() const override { return positions; }
    virtual Vector* getPositionData() const override { return positionData.data(); }
    constexpr const Array<StaticMatData>& getStaticMeshes() const { return staticData; }

  private:
    virtual void resizeBuffers() override;
    virtual void updateBuffers() override;
    Array<StaticMatData> staticData;

    std::mutex mutex;
    Gfx::OShaderBuffer positions;
    Array<Vector> positionData;
    Gfx::OShaderBuffer texCoords[MAX_TEXCOORDS];
    Array<Vector2> texCoordsData[MAX_TEXCOORDS];
    Gfx::OShaderBuffer normals;
    Array<Vector> normalData;
    Gfx::OShaderBuffer tangents;
    Array<Vector> tangentData;
    Gfx::OShaderBuffer biTangents;
    Array<Vector> biTangentData;
    Gfx::OShaderBuffer colors;
    Array<Vector> colorData;
    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
};
} // namespace Seele
