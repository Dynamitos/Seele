#include "FontAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"

using namespace Seele;

FontAsset::FontAsset() {}

FontAsset::FontAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {
    FT_Error error = FT_Init_FreeType(&ft);
    assert(!error);
}

FontAsset::~FontAsset() {
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void FontAsset::save(ArchiveBuffer& buffer) const { Serialization::save(buffer, ttfFile); }

void FontAsset::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, ttfFile);
    graphics = buffer.getGraphics();
    loadFace();
}

void FontAsset::loadFace() {
    FT_Error error = FT_New_Memory_Face(ft, ttfFile.data(), ttfFile.size(), 0, &face);
    assert(!error);
}

void FontAsset::loadFontSize(uint32 fontSize) {
    FT_Set_Pixel_Sizes(face, 0, fontSize);
    fontSizes[fontSize].ascent = face->ascender;
    fontSizes[fontSize].descent = face->descender;
    fontSizes[fontSize].linegap = 0;
    for (uint32 c = 0; c < 256; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER | FT_LOAD_COLOR)) {
            continue;
        }
        FontAsset::Glyph& glyph = fontSizes[fontSize].glyphs[c];
        glyph.size = IVector2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        glyph.bearing = IVector2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        glyph.advance = face->glyph->advance.x;
        glyph.textureData.resize(glyph.size.x * glyph.size.y);
        if (glyph.textureData.size() == 0) {
            glyph.size.x = 1;
            glyph.size.y = 1;
            glyph.textureData.add(0);
            // load a single transparent pixel, so that we dont have to handle empty textures later
        } else {
            std::memcpy(glyph.textureData.data(), face->glyph->bitmap.buffer, glyph.textureData.size());
        }
        glyph.texture = graphics->createTexture2D(TextureCreateInfo{
            .sourceData =
                {
                    .size = glyph.textureData.size(),
                    .data = glyph.textureData.data(),
                },
            .format = Gfx::SE_FORMAT_R8_UNORM,
            .width = glyph.size.x,
            .height = glyph.size.y,
            .name = "FontGlyph",
        });
    }
}