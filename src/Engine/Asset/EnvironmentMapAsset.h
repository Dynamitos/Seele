#pragma once
#include "Asset.h"
#include "Graphics/Texture.h"

namespace Seele {
class EnvironmentMapAsset : public Asset {
  public:
    static constexpr uint64 IDENTIFIER = 0x80;
    EnvironmentMapAsset();
    EnvironmentMapAsset(std::string_view folderPath, std::string_view name);
    virtual ~EnvironmentMapAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
    Gfx::PTextureCube getSkybox() const { return skybox; }
    Gfx::PTextureCube getIrradianceMap() const { return irradianceMap; }
    Gfx::PTextureCube getPrefilteredMap() const { return prefilteredMap; }
    Gfx::PTexture2D getBrdfLUT() const { return brdfLUT; }
    Gfx::PSampler getLUTSampler() const { return lutSampler; }
  private:
    Gfx::OTextureCube skybox;
    Gfx::OTextureCube irradianceMap;
    Gfx::OTextureCube prefilteredMap;
    Gfx::PTexture2D brdfLUT;
    Gfx::PSampler lutSampler;
    friend class EnvironmentLoader;
};
} // namespace Seele