#pragma once
#include "Graphics/Command.h"
#include "Graphics/Initializer.h"
#include "Math/Vector.h"
#include "VertexData.h"
#include "entt/entt.hpp"

namespace Seele {
class StaticMeshVertexData : public VertexData {
  public:
    using NormalType = Vector;
    using TangentType = Vector;
    using BiTangentType = Vector;
    using TexCoordType = U16Vector2;
    using ColorType = U16Vector;

    StaticMeshVertexData();
    virtual ~StaticMeshVertexData();
    static StaticMeshVertexData* getInstance();
    void loadTexCoords(uint64 offset, uint64 index, const Array<TexCoordType>& data);
    void loadNormals(uint64 offset, const Array<NormalType>& data);
    void loadTangents(uint64 offset, const Array<TangentType>& data);
    void loadBitangents(uint64 offset, const Array<BiTangentType>& data);
    void loadColors(uint64 offset, const Array<ColorType>& data);
    virtual void serializeMesh(MeshId id, ArchiveBuffer& buffer) override;
    virtual uint64 deserializeMesh(MeshId id, ArchiveBuffer& buffer) override;
    virtual void init(Gfx::PGraphics graphics) override;
    virtual void destroy() override;
    virtual Gfx::PDescriptorLayout getVertexDataLayout() override { return descriptorLayout; }
    virtual Gfx::PDescriptorSet getVertexDataSet() override { return descriptorSet; }
    virtual std::string getTypeName() const override { return "StaticMeshVertexData"; }

  private:
    virtual void resizeBuffers() override;
    virtual void updateBuffers() override;

    Gfx::OShaderBuffer texCoords[MAX_TEXCOORDS];
    constexpr static const char* TEXCOORDS_NAME = "texCoords";
    Gfx::OShaderBuffer normals;
    constexpr static const char* NORMALS_NAME = "normals";
    Gfx::OShaderBuffer tangents;
    constexpr static const char* TANGENTS_NAME = "tangents";
    Gfx::OShaderBuffer biTangents;
    constexpr static const char* BITANGENTS_NAME = "biTangents";
    Gfx::OShaderBuffer colors;
    constexpr static const char* COLORS_NAME = "colors";
    Array<TexCoordType> texData[MAX_TEXCOORDS];
    Array<NormalType> norData;
    Array<TangentType> tanData;
    Array<BiTangentType> bitData;
    Array<ColorType> colData;
    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
};
} // namespace Seele
