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
    void createFromMemory(Array<uint8> memory, Gfx::PGraphics graphics);
    void setTexture(Gfx::PTexture _texture)
    {
        texture = _texture;
    }
    Gfx::PTexture getTexture()
    {
        return texture;
    }
private:
    Array<uint8> textureData;
    Gfx::PTexture texture;
    friend class TextureLoader;
};
DEFINE_REF(TextureAsset)
} // namespace Seele
