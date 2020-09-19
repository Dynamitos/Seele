#include "MaterialLoader.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialLoader::MaterialLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    placeholderMaterial = new Material(std::filesystem::absolute("./shaders/Placeholder.asset"));
    placeholderMaterial->compile();
    graphics->getShaderCompiler()->registerMaterial(placeholderMaterial);
}

MaterialLoader::~MaterialLoader()
{
}

PMaterial MaterialLoader::queueAsset(const std::filesystem::path& filePath)
{
    PMaterial result = new Material(filePath);
    result->compile();
    graphics->getShaderCompiler()->registerMaterial(result);
    // TODO: There is actually no real reason to import a standalone material,
    // maybe in the future there could be a substance loader or something
    return result;
}
