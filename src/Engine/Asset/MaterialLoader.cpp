#include "MaterialLoader.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialLoader::MaterialLoader(Gfx::PGraphics graphics)
{
    placeholderMaterial = new Material("shaders/Placeholder.semat");
    placeholderMaterial->compile();
}

MaterialLoader::~MaterialLoader()
{
}

PMaterial MaterialLoader::queueAsset(const std::string& filePath)
{
    PMaterial result = new Material(filePath);
    //TODO
    return result;
}
