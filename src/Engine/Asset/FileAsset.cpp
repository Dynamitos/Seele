#include "FileAsset.h"

using namespace Seele;

FileAsset::FileAsset()
    : path("")
    , name("")
{
}

FileAsset::FileAsset(const std::string &fullPath)
    : path(fullPath)
{
    name = fullPath.substr(fullPath.find_last_of('\\')+1);
}

FileAsset::FileAsset(const std::string &directory, const std::string &name)
    : path(directory+name)
    , name(name)
{
}

FileAsset::~FileAsset()
{
}

std::ifstream &FileAsset::getReadStream()
{
    if(inStream.is_open())
    {
        return inStream;
    }
    inStream.open(path);
    return inStream;
}

std::ofstream &FileAsset::getWriteStream()
{
    if(outStream.is_open())
    {
        return outStream;
    }
    outStream.open(path);
    return outStream;
}

void FileAsset::rename(const std::string& newName) 
{
    if(inStream.is_open())
    {
        inStream.close();
    }
    if(outStream.is_open())
    {
        outStream.flush();
        outStream.close();
    }
    path.replace(path.find_last_of(name), name.size(), newName);
    name = newName;
    inStream.open(path);
    outStream.open(path);
}