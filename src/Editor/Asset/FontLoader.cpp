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
void FontLoader::import(FontImportArgs args, PFontAsset asset) {
    std::ifstream stream(args.filePath.c_str(), std::ios::binary | std::ios::ate);
    Array<uint8> ttfFile(stream.tellg());
    stream.seekg(0);
    stream.read((char*)ttfFile.data(), ttfFile.size());
    asset->ttfFile = std::move(ttfFile);
    asset->graphics = graphics;
    asset->loadFace();

    AssetRegistry::saveAsset(asset, FontAsset::IDENTIFIER, asset->getFolderPath(), asset->getName());

    asset->setStatus(Asset::Status::Ready);
}