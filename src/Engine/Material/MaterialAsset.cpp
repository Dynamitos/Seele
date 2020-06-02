#include "MaterialAsset.h"

using namespace Seele;

MaterialAsset::MaterialAsset()
{
}

MaterialAsset::MaterialAsset(const std::string& directory, const std::string& name) 
    : Asset(directory, name)
{
}

MaterialAsset::MaterialAsset(const std::string& fullPath) 
    : Asset(fullPath)
{
}

MaterialAsset::~MaterialAsset()
{
}
