#include "FontLoader.h"
#include "Graphics/Graphics.h"
#include "FontAsset.h"
#include "AssetRegistry.h"

using namespace Seele;

FontLoader::FontLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    
}

FontLoader::~FontLoader()
{
}

void FontLoader::importAsset(const std::filesystem::path& filePath)
{
    std::filesystem::path assetPath = filePath.filename();
    assetPath.replace_extension("asset");
    PFontAsset asset = new FontAsset(assetPath.generic_string());
    std::error_code code;
    std::filesystem::copy_file(filePath, asset->getFullPath(), code);
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerFont(asset);
    import(filePath, asset);
}

void FontLoader::import(std::filesystem::path path, PFontAsset asset)
{
    asset->load();
    AssetRegistry::get().registerFont(asset);
}