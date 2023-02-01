#include "Asset.h"
#include "AssetRegistry.h"

using namespace Seele;

Asset::Asset()
    : fullPath("")
    , name("")
    , parentDir("")
    , extension("")
    , status(Status::Uninitialized)
    , byteSize(0)
{
}
Asset::Asset(const std::filesystem::path& path)
    : status(Status::Uninitialized)
{
    if(path.is_absolute())
    {
        fullPath = path;
    }
    else
    {
        fullPath = AssetRegistry::getRootFolder();
        fullPath.append(path.generic_string());
    }
    
    fullPath.make_preferred();
    parentDir = fullPath.parent_path();
    name = fullPath.stem();
    extension = fullPath.extension();
}

Asset::Asset(const std::string &directory, const std::string &fileName)
    : Asset(std::filesystem::path(directory + fileName))
{
}

Asset::~Asset()
{
}

std::ifstream Asset::getReadStream() const
{
    return std::ifstream(fullPath, std::ios::binary);
}

std::ofstream Asset::getWriteStream() const
{
    return std::ofstream(fullPath, std::ios::binary);
}

std::string Asset::getFileName() const
{
    return name.generic_string();
}
std::string Asset::getFullPath() const
{
    return fullPath.generic_string();
}
std::string Asset::getExtension() const
{
    return extension.generic_string();
}