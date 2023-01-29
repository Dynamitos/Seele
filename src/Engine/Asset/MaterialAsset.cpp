#include "MaterialAsset.h"
#include "Material/Material.h"
#include "Graphics/Graphics.h"

using namespace Seele;

MaterialAsset::MaterialAsset()
{
}

MaterialAsset::MaterialAsset(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialAsset::MaterialAsset(const std::filesystem::path& fullPath) 
    : Asset(fullPath)
{
}

MaterialAsset::~MaterialAsset()
{
}


void MaterialAsset::save(Gfx::PGraphics graphics)
{
}

void MaterialAsset::load(Gfx::PGraphics graphics)
{
    
}

