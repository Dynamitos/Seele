#include "FontLoader.h"
#include "Asset/AssetRegistry.h"
#include "Asset/FontAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Resources.h"
#include "Graphics/Texture.h"
#include <ft2build.h>

#include FT_FREETYPE_H
#include <fstream>
#include <iostream>


using namespace Seele;

FontLoader::FontLoader(Gfx::PGraphics graphics) : graphics(graphics) {}

FontLoader::~FontLoader() {}

void FontLoader::importAsset(FontImportArgs args) {
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    OFontAsset asset = new FontAsset(args.importPath, assetPath.stem().string());
    asset->setStatus(Asset::Status::Loading);
    // the registry takes ownership, but we need to edit the reference
    PFontAsset ref = asset;
    AssetRegistry::get().registerFont(std::move(asset));
    import(args, ref);
}

// in case of the space character there is no bitmap
// so we create a single pixel empty texture
uint8 transparentPixel = 0;
void FontLoader::import(FontImportArgs args, PFontAsset asset) {
    FT_Library ft;
    FT_Error error = FT_Init_FreeType(&ft);
    assert(!error);
    FT_Face face;
    error = FT_New_Face(ft, args.filePath.string().c_str(), 0, &face);
    assert(!error);
    FT_Set_Pixel_Sizes(face, 0, 48);
    Array<Gfx::OTexture2D> usedTextures;
    for (uint32 c = 0; c < 256; ++c) {
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
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
        imageData.sourceData.data = face->glyph->bitmap.buffer;
        imageData.sourceData.size = imageData.width * imageData.height;
        if (imageData.width == 0 || imageData.height == 0) {
            glyph.size.x = 1;
            glyph.size.y = 1;
            glyph.bearing.x = 0;
            glyph.bearing.y = 0;
            imageData.width = 1;
            imageData.height = 1;
            imageData.sourceData.size = sizeof(uint8);
            imageData.sourceData.data = &transparentPixel;
        }
        glyph.textureIndex = usedTextures.size();
        usedTextures.add(graphics->createTexture2D(imageData));
    }
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
    asset->setUsedTextures(std::move(usedTextures));

    AssetRegistry::saveAsset(asset, FontAsset::IDENTIFIER, asset->getFolderPath(), asset->getName());

    asset->setStatus(Asset::Status::Ready);
}