#pragma once
#include "Asset.h"
#include "TextureAsset.h"

namespace Seele {
class LevelAsset : public Asset {
  public:
    static constexpr uint64 IDENTIFIER = 0x40;
    LevelAsset();
    LevelAsset(std::string_view folderPath, std::string_view name);
    virtual ~LevelAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;

  private:
    Array<OTextureAsset> lightMaps;
};
} // namespace Seele
