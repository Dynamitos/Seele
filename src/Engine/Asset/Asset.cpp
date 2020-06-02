#include "Asset.h"

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
    : fullPath(std::filesystem::absolute(path))
    , status(Status::Uninitialized)
{
    fullPath.make_preferred();
    parentDir = fullPath.parent_path();
    name = fullPath.stem();
    extension = fullPath.extension();
}
Asset::Asset(const std::string &fullPath)
    : Asset(std::filesystem::path(fullPath))
{
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

std::string Asset::getFileName()
{
    return name.generic_string();
}
std::string Asset::getFullPath()
{
    return fullPath.generic_string();
}
std::string Asset::getExtension()
{
    return extension.generic_string();
}