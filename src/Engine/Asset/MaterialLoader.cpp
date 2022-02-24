#include "MaterialLoader.h"
#include "Graphics/Graphics.h"
#include "Material/MaterialAsset.h"
#include "AssetRegistry.h"

using namespace Seele;

MaterialLoader::MaterialLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    placeholderMaterial = new MaterialAsset(std::filesystem::absolute("./shaders/Placeholder.asset"));
    placeholderMaterial->load();
    graphics->getShaderCompiler()->registerMaterial(placeholderMaterial);
}

MaterialLoader::~MaterialLoader()
{
}

void MaterialLoader::importAsset(const std::filesystem::path& name)
{
    std::filesystem::path assetPath = name.filename();
    assetPath.replace_extension("asset");
    PMaterialAsset asset = new MaterialAsset(assetPath.generic_string());
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerMaterial(asset);
    import(name, asset);
}

void MaterialLoader::import(std::filesystem::path, PMaterialAsset asset)
{
    asset->load();
    graphics->getShaderCompiler()->registerMaterial(asset);
    AssetRegistry::get().registerMaterial(asset);
    //co_return;
}

PMaterialAsset MaterialLoader::getPlaceHolderMaterial() 
{
    return placeholderMaterial;
}
