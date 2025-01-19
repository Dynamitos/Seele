#pragma once
#include "Asset.h"
#include "Containers/Map.h"
#include "Graphics/Texture.h"
#include "Math/Math.h"
#include <freetype/freetype.h>

namespace Seele {
class FontAsset : public Asset {
  public:
    static constexpr uint64 IDENTIFIER = 0x10;
    FontAsset();
    FontAsset(std::string_view folderPath, std::string_view name);
    virtual ~FontAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;

    struct Glyph {
        UVector2 size = {};
        UVector2 bearing = {};
        uint32 advance = 0;
        Array<uint8> textureData;
        Gfx::OTexture2D texture = nullptr;
        void save(ArchiveBuffer& buffer) const;
        void load(ArchiveBuffer& buffer);
    };
    struct FontData {
        int32 ascent;
        int32 descent;
        int32 linegap;
        Map<uint32, Glyph> glyphs;
    };
    const Glyph& getGlyphData(char c, uint32 fontSize) {
        if (!fontSizes.contains(fontSize))
            loadFontSize(fontSize);
        return fontSizes[fontSize].glyphs[c];
    }
    const FontData& getFontData(uint32 fontSize) { return fontSizes[fontSize]; }
    void loadFace();

  private:
    void loadFontSize(uint32 fontSize);
    Gfx::PGraphics graphics;
    FT_Library ft;
    FT_Face face;
    Array<uint8> ttfFile;
    Map<uint32, FontData> fontSizes;
    friend class FontLoader;
};
DECLARE_REF(FontAsset)
} // namespace Seele
