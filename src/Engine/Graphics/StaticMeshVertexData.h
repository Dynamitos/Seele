#pragma once
#include "Graphics/Command.h"
#include "Graphics/Initializer.h"
#include "Math/Vector.h"
#include "VertexData.h"
#include "entt/entt.hpp"


namespace Seele {
class StaticMeshVertexData : public VertexData {
  public:
    StaticMeshVertexData();
    virtual ~StaticMeshVertexData();
    static StaticMeshVertexData* getInstance();
    void loadPositions(uint64 offset, const Array<Vector4>& data);
    void loadTexCoords(uint64 offset, uint64 index, const Array<Vector2>& data);
    void loadNormals(uint64 offset, const Array<Vector4>& data);
    void loadTangents(uint64 offset, const Array<Vector4>& data);
    void loadBiTangents(uint64 offset, const Array<Vector4>& data);
    void loadColors(uint64 offset, const Array<Vector4>& data);
    virtual void serializeMesh(MeshId id, uint64 numVertices, ArchiveBuffer& buffer) override;
    virtual uint64 deserializeMesh(MeshId id, ArchiveBuffer& buffer) override;
    virtual void init(Gfx::PGraphics graphics) override;
    virtual void destroy() override;
    virtual void bindBuffers(Gfx::PRenderCommand command) override;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() override;
    virtual Gfx::PDescriptorSet getVertexDataSet() override;
    virtual std::string getTypeName() const override { return "StaticMeshVertexData"; }
    virtual Gfx::PShaderBuffer getPositionBuffer() const override { return positions; }

  private:
    virtual void resizeBuffers() override;
    virtual void updateBuffers() override;

    Gfx::OShaderBuffer positions;
    Gfx::OShaderBuffer texCoords[MAX_TEXCOORDS];
    Gfx::OShaderBuffer normals;
    Gfx::OShaderBuffer tangents;
    Gfx::OShaderBuffer biTangents;
    Gfx::OShaderBuffer colors;
    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
};
} // namespace Seele
