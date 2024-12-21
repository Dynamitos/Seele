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
    void loadPositions(uint64 offset, const Array<Vector>& data);
    void loadTexCoords(uint64 offset, uint64 index, const Array<U16Vector2>& data);
    void loadNormals(uint64 offset, const Array<uint32>& data);
    void loadColors(uint64 offset, const Array<U16Vector>& data);
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
    Gfx::OShaderBuffer colors;
    Array<Vector> posData;
    Array<U16Vector2> texData[MAX_TEXCOORDS];
    Array<Quaternion> norData;
    Array<U16Vector4> colData;
    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
};
} // namespace Seele
