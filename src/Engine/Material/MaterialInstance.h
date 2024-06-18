#pragma once
#include "Asset/MaterialAsset.h"
#include "Graphics/Buffer.h"
#include "Material.h"

namespace Seele {
struct MaterialOffsets {
    uint32 textureOffset;
    uint32 samplerOffset;
    uint32 floatOffset;
};
class MaterialInstance {
  public:
    MaterialInstance();
    MaterialInstance(uint64 id, Gfx::PGraphics graphics, Array<OShaderExpression>& expressions, Array<std::string> params,
                     uint32 numTextures, uint32 numSamplers, uint32 numFloats);
    ~MaterialInstance();
    void updateDescriptor();
    PMaterial getBaseMaterial() const { return baseMaterial->getMaterial(); }
    uint64 getId() const { return id; }

    void setBaseMaterial(PMaterialAsset asset);
    MaterialOffsets getMaterialOffsets() const {
        return MaterialOffsets{
            .textureOffset = texturesOffset,
            .samplerOffset = samplersOffset,
            .floatOffset = floatBufferOffset,
        };
    }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

  private:
    Gfx::PGraphics graphics;
    Array<OShaderParameter> parameters;
    uint32 numTextures;
    uint32 numSamplers;
    uint32 numFloats;
    uint32 texturesOffset;
    uint32 samplersOffset;
    uint32 floatBufferOffset;
    Gfx::PDescriptorSet descriptor;
    PMaterialAsset baseMaterial;
    uint64 id;
};
DEFINE_REF(MaterialInstance)
} // namespace Seele