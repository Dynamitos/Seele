#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
class FontAsset : public Asset
{
public:
    FontAsset();
    FontAsset(const std::string& directory, const std::string& name);
    FontAsset(const std::filesystem::path& fullPath);
    virtual ~FontAsset();
    virtual void save() override;
    virtual void load() override;

    struct Glyph
    {
        Gfx::PTexture2D texture;
        Math::IVector2 size;
        Math::IVector2 bearing;
        uint32 advance;
    };
    const std::map<uint32, Glyph> getGlyphData() const { return glyphs; }
private:
    std::map<uint32, Glyph> glyphs;
};
DECLARE_REF(FontAsset)
} // namespace Seele
