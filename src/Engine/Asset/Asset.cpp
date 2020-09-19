#include "Asset.h"
#include "AssetRegistry.h"

using namespace Seele;

Asset::Asset()
    : fullPath("")
    , name("")
    , parentDir("")
    , extension("")
    , status(Status::Uninitialized)
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

std::ifstream &Asset::getReadStream()
{
    if(inStream.is_open())
    {
        return inStream;
    }
    inStream.open(fullPath);
    return inStream;
}

std::ofstream &Asset::getWriteStream()
{
    if(outStream.is_open())
    {
        return outStream;
    }
    outStream.open(fullPath);
    return outStream;
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