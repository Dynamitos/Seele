#pragma once
#include "Graphics/Descriptor.h"
#include "ShaderExpression.h"
#include <atomic>


namespace Seele {
DECLARE_REF(MaterialInstance)
DECLARE_NAME_REF(Gfx, Texture2D)
DECLARE_NAME_REF(Gfx, Sampler)
class Material {
  public:
    Material();
    Material(Gfx::PGraphics graphics, uint32 numTextures, uint32 numSamplers, uint32 numFloats, bool twoSided, float opacity,
             std::string materialName, Array<OShaderExpression> expressions, Array<std::string> parameter, MaterialNode brdf);
    ~Material();
    static void init(Gfx::PGraphics graphics);
    static void destroy();
    static Gfx::PDescriptorLayout getDescriptorLayout() { return layout; }
    static Gfx::PDescriptorSet getDescriptorSet() { return set; }
    static void updateDescriptor();
    static void updateTexture(uint32 index, Gfx::PTexture2D texture);
    static void updateSampler(uint32 index, Gfx::PSampler sampler);
    static void updateFloatData(uint32 offset, uint32 numFloats, float* data);
    static uint32 addTextures(uint32 numTextures);
    static uint32 addSamplers(uint32 numSamplers);
    static uint32 addFloats(uint32 numFloats);

    OMaterialInstance instantiate();
    const std::string& getName() const { return materialName; }
    const std::string& getProfile() const { return brdf.profile; }

    bool isTwoSided() const;
    bool hasTransparency() const;
    float getOpacity() const;

    constexpr uint64 getId() const { return materialId; }
    static constexpr const PMaterial findMaterialById(uint64 id) { return materials[id]; }

    void save(ArchiveBuffer& buffer) const;
    void load(ArchiveBuffer& buffer);

    void compile();

    uint64 getCPUSize() const;
    uint64 getGPUSize() const;

  private:
    Gfx::PGraphics graphics;
    uint32 numTextures;
    uint32 numSamplers;
    uint32 numFloats;
    uint64 instanceId;
    uint64 materialId;
    std::string materialName;
    bool twoSided;
    float opacity;
    Array<OShaderExpression> codeExpressions;
    Array<std::string> parameters;
    MaterialNode brdf;
    static Array<Gfx::PTexture2D> textures;
    static Array<Gfx::PSampler> samplers;
    static Gfx::OShaderBuffer floatBuffer;
    static Array<float> floatData;
    static Gfx::ODescriptorLayout layout;
    static Gfx::ODescriptorSet set;
    static std::atomic_uint64_t materialIdCounter;
    static Array<PMaterial> materials;
};
DEFINE_REF(Material)

} // namespace Seele
