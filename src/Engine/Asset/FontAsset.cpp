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
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft);
    hb_font_destroy(hb_font);
    hb_face_destroy(hb_face);
    hb_blob_destroy(blob);
}

void FontAsset::save(ArchiveBuffer& buffer) const { Serialization::save(buffer, ttfFile); }

void FontAsset::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, ttfFile);
    graphics = buffer.getGraphics();
    loadFace();
}

void FontAsset::loadFace() {
    FT_Error error = FT_New_Memory_Face(ft, (unsigned char*)ttfFile.data(), ttfFile.size(), 0, &ft_face);
    assert(!error);
    blob = hb_blob_create(ttfFile.data(), ttfFile.size(), HB_MEMORY_MODE_DUPLICATE, nullptr, nullptr);
    hb_face = hb_face_create(blob, 0);
    hb_font = hb_font_create(hb_face);
}
UVector2 FontAsset::shapeText(std::string_view view, uint32 fontSize, Array<RenderGlyph>& render) {
    loadGlyphs(fontSize);
    hb_buffer_t* buf = hb_buffer_create();
    hb_buffer_add_utf8(buf, view.data(), view.size(), 0, -1);
    hb_buffer_set_language(buf, hb_language_from_string("en", -1));
    hb_buffer_set_script(buf, HB_SCRIPT_LATIN);
    hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
    hb_shape(hb_font, buf, nullptr, 0);
    uint32 num_glyphs;
    hb_glyph_info_t* glyphs = hb_buffer_get_glyph_infos(buf, &num_glyphs);
    hb_glyph_position_t* positions = hb_buffer_get_glyph_positions(buf, &num_glyphs);
    Vector2 min = Vector2(std::numeric_limits<float>::max());
    Vector2 max = Vector2(std::numeric_limits<float>::lowest());
    int32 xppem, yppem;
    hb_font_get_scale(hb_font, &xppem, &yppem);
    Vector2 cursor = Vector2(0);
    for (uint32 i = 0; i < num_glyphs; ++i) {
        hb_codepoint_t glyphid = glyphs[i].codepoint;
        float xOffset = positions[i].x_offset * fontSize / xppem;
        float yOffset = positions[i].y_offset * fontSize / yppem;
        float xAdvance = positions[i].x_advance * fontSize / xppem;
        float yAdvance = positions[i].y_advance * fontSize / yppem;
        Vector2 position = cursor + Vector2(xOffset, yOffset);
        Vector2 dimensions = fontSizes[fontSize].glyphs[glyphid].size;
        render.add(RenderGlyph{
            .texture = fontSizes[fontSize].glyphs[glyphid].texture,
            .position = position,
            .dimensions = dimensions,
        });
        cursor += Vector2(xAdvance, yAdvance);
        min = Vector2(std::min(min.x, position.x), std::min(min.y, position.y));
        max = Vector2(std::max(max.x, position.x + dimensions.x), std::max(min.y, position.y + dimensions.y));
    }
    hb_buffer_destroy(buf);
    return UVector2(max - min);
}

void FontAsset::loadGlyphs(uint32 fontSize) {
    FT_Set_Pixel_Sizes(ft_face, 0, fontSize);
    for (uint32 i = 0; i < ft_face->num_glyphs; ++i) {
        assert(FT_Load_Glyph(ft_face, i, FT_LOAD_RENDER | FT_LOAD_COLOR) == 0);
        FontAsset::Glyph& glyph = fontSizes[fontSize].glyphs[i];
        glyph.size = IVector2(ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows);
        glyph.bearing = IVector2(ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top);
        glyph.advance = ft_face->glyph->advance.x;
        glyph.textureData.resize(glyph.size.x * glyph.size.y);
        if (glyph.textureData.size() == 0) {
            glyph.size.x = 1;
            glyph.size.y = 1;
            glyph.textureData.add(0);
            // load a single transparent pixel, so that we dont have to handle empty textures later
        } else {
            std::memcpy(glyph.textureData.data(), ft_face->glyph->bitmap.buffer, glyph.textureData.size());
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