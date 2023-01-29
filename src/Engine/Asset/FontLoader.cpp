#include "FontLoader.h"
#include "Graphics/Graphics.h"
#include "FontAsset.h"
#include "AssetRegistry.h"
#include "Graphics/GraphicsResources.h"
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace Seele;

FontLoader::FontLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    
}

FontLoader::~FontLoader()
{
}

void FontLoader::importAsset(const std::filesystem::path& filePath, const std::string& importPath)
{
    std::filesystem::path assetPath = filePath.filename();
    assetPath.replace_extension("asset");
    PFontAsset asset = new FontAsset(assetPath.generic_string());
    std::error_code code;
    std::filesystem::copy_file(filePath, asset->getFullPath(), code);
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerFont(asset, importPath);
    import(filePath, asset);
}

// in case of the space character there is no bitmap
// so we create a single pixel empty texture
uint8 transparentPixel = 0;
void FontLoader::import(std::filesystem::path path, PFontAsset asset)
{
    FT_Library ft;
    FT_Error error = FT_Init_FreeType(&ft);
    assert(!error);
    FT_Face face;
    error = FT_New_Face(ft, asset->getFullPath().c_str(), 0, &face);
    assert(!error);
    FT_Set_Pixel_Sizes(face, 0, 48);
    for(uint32 c = 0; c < 256; ++c)
    {
        if(FT_Load_Char(face, c, FT_LOAD_RENDER))
        {
            std::cout << "error loading " << (char)c << std::endl;
            continue;
        }
        FontAsset::Glyph& glyph = asset->glyphs[c];
        glyph.size = IVector2(face->glyph->bitmap.width, face->glyph->bitmap.rows);
        glyph.bearing = IVector2(face->glyph->bitmap_left, face->glyph->bitmap_top);
        glyph.advance = face->glyph->advance.x;
        TextureCreateInfo imageData;
        imageData.format = Gfx::SE_FORMAT_R8_UINT;
        imageData.width = face->glyph->bitmap.width;
        imageData.height = face->glyph->bitmap.rows;
        imageData.resourceData.data = face->glyph->bitmap.buffer;
        imageData.resourceData.size = imageData.width * imageData.height;
        if(imageData.width == 0 || imageData.width == 0)
        {
            glyph.size.x = 1;
            glyph.size.y = 1;
            glyph.bearing.x = 0;
            glyph.bearing.y = 0;
            imageData.width = 1;
            imageData.height = 1;
            imageData.resourceData.size = sizeof(uint8);
            imageData.resourceData.data = &transparentPixel;
        }
        glyph.texture = graphics->createTexture2D(imageData);
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    asset->setStatus(Asset::Status::Ready);
}