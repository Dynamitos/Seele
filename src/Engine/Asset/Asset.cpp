#include "Asset.h"
#include "AssetRegistry.h"
#include "Graphics/Graphics.h"

using namespace Seele;

Asset::Asset()
    : folderPath("")
    , name("")
    , status(Status::Uninitialized)
    , byteSize(0)
{
}
Asset::Asset(std::string_view _folderPath, std::string_view _name)
    : status(Status::Uninitialized)
    , folderPath(_folderPath)
    , name(_name)
{
    if (folderPath.empty())
    {
        assetId = name;
    }
    else
    {
        assetId = folderPath + "/" + name;
    }
}


Asset::~Asset()
{
}

std::string Asset::getFolderPath() const
{
    return folderPath;
}

std::string Asset::getName() const
{
    return name;
}

std::string Asset::getAssetIdentifier() const
{
    return assetId;
}
