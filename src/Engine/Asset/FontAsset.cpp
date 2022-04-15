#include "FontAsset.h"
#include "Graphics/GraphicsResources.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include "Window/WindowManager.h"

using namespace Seele;

FontAsset::FontAsset()
{
}

FontAsset::FontAsset(const std::string& directory, const std::string& name)
    : Asset(directory, name)
{
}

FontAsset::FontAsset(const std::filesystem::path& fullPath)
    : Asset(fullPath)
{
}

FontAsset::~FontAsset()
{
    
}

void FontAsset::save()
{
    assert(false && "Cannot save font files");
}

void FontAsset::load()
{
    FT_Library ft;
    assert(!FT_Init_FreeType(&ft));
    FT_Face face;
    assert(!FT_New_Face(ft, getFullPath().c_str(), 0, &face));
    FT_Set_Pixel_Sizes(face, 0, 48);
    for(uint8 c = 0; c < 128; ++c)
    {
        Gfx::PGraphics graphics = WindowManager::getGraphics();
        if(FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "error loading " << (char)c << std::endl;
        }
        TextureCreateInfo imageData;
        imageData.format = Gfx::SE_FORMAT_R8_UINT;
        imageData.width = face->glyph->bitmap.width;
        imageData.height = face->glyph->bitmap.rows;
        imageData.resourceData.data = face->glyph->bitmap.buffer;
        imageData.resourceData.size = imageData.width * imageData.height;
        Glyph& glyph = glyphs[c];
        glyph.texture = graphics->createTexture2D(imageData);
        glyph.size = IVector2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        glyph.bearing = IVector2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        glyph.advance = face->glyph->advance.x;
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}