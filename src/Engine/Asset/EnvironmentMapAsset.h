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
  private:
    Gfx::OTextureCube diffuseMap;
    Gfx::OTextureCube specularMap;
    friend class EnvironmentLoader;
};
} // namespace Seele