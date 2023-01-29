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
    virtual void save(Gfx::PGraphics graphics) override;
    virtual void load(Gfx::PGraphics graphics) override;

    struct Glyph
    {
        Gfx::PTexture2D texture;
        IVector2 size;
        IVector2 bearing;
        uint32 advance;
    };
    const std::map<uint32, Glyph> getGlyphData() const { return glyphs; }
private:
    std::map<uint32, Glyph> glyphs;
    friend class FontLoader;
};
DECLARE_REF(FontAsset)
} // namespace Seele
