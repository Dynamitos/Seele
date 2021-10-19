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

PMaterialAsset MaterialLoader::queueAsset(const std::filesystem::path& filePath)
{
    PMaterialAsset result = new MaterialAsset(filePath);
    result->load();
    graphics->getShaderCompiler()->registerMaterial(result);
    AssetRegistry::get().registerMaterial(result);
    // TODO: There is actually no real reason to import a standalone material,
    // maybe in the future there could be a substance loader or something
    return result;
}

PMaterialAsset MaterialLoader::getPlaceHolderMaterial() 
{
    return placeholderMaterial;
}
