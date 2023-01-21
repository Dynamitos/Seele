#include "MaterialInstanceAsset.h"
#include "Material/MaterialInstance.h"
#include "Material/Material.h"

using namespace Seele;

MaterialInstanceAsset::MaterialInstanceAsset()
{
}

MaterialInstanceAsset::MaterialInstanceAsset(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialInstanceAsset::MaterialInstanceAsset(const std::filesystem::path& fullPath) 
    : Asset(fullPath)
{
}

MaterialInstanceAsset::~MaterialInstanceAsset()
{
}


void MaterialInstanceAsset::save()
{
}

void MaterialInstanceAsset::load()
{
}
