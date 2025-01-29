#pragma once
#include "Asset.h"
#include "Containers/Map.h"
#include "Graphics/Texture.h"
#include "Math/Math.h"
#include <freetype/freetype.h>
#include <harfbuzz/hb.h>

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
        Map<uint32, Glyph> glyphs;
    };
    void loadFace();

    struct RenderGlyph {
        Gfx::PTexture2D texture;
        UVector2 position;
        UVector2 dimensions;
    };
    UVector2 shapeText(std::string_view view, uint32 fontSize, Array<RenderGlyph>& render);

  private:
    void loadGlyphs(uint32 fontSize);
    Gfx::PGraphics graphics;
    hb_blob_t* blob;
    hb_face_t* hb_face;
    hb_font_t* hb_font;
    FT_Library ft;
    FT_Face ft_face;
    Array<char> ttfFile;
    Map<uint32, FontData> fontSizes;
    friend class FontLoader;
};
DECLARE_REF(FontAsset)
} // namespace Seele
