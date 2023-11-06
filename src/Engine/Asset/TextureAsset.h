#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture)
class TextureAsset : public Asset
{
public:
    static constexpr uint64 IDENTIFIER = 0x1;
    TextureAsset();
    TextureAsset(std::string_view folderPath, std::string_view name);
    virtual ~TextureAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
    void setTexture(Gfx::OTexture _texture);
    Gfx::PTexture getTexture()
    {
        return texture;
    }
private:
    Gfx::OTexture texture;
    friend class TextureLoader;
};
DEFINE_REF(TextureAsset)
} // namespace Seele
