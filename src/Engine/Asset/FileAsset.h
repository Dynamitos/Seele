#pragma once
#include "Asset.h"
#include <fstream>

namespace Seele
{
class FileAsset
{
public:
    FileAsset();
    FileAsset(const std::string& directory, const std::string& name);
    FileAsset(const std::string& fullPath);
    virtual ~FileAsset();
    std::ifstream& getReadStream();
    std::ofstream& getWriteStream();
    void rename(const std::string& newName);
private:
    std::string name;
    std::string path;
    uint32 byteSize;
    std::ifstream inStream;
    std::ofstream outStream;
};
}