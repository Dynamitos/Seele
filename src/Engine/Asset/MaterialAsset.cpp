#include "MaterialAsset.h"
#include "Material/Material.h"

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


void MaterialAsset::save()
{
}

void MaterialAsset::load()
{
    
}


void MaterialAsset::beginFrame() 
{
}

void MaterialAsset::endFrame() 
{
}
