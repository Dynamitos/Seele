#pragma once
#include "MinimalEngine.h"

namespace Seele
{
class Asset
{
public:
    enum class Status
    {
        Uninitialized,
        Loading,
        Ready
    };
    Asset();
    Asset(const std::filesystem::path& path);
    Asset(const std::string& directory, const std::string& name);
    Asset(const std::string& fullPath);
    virtual ~Asset();
    std::ifstream& getReadStream();
    std::ofstream& getWriteStream();
    
    // returns the name of the file, without extension
    std::string getFileName();
    // returns the full absolute path, from root to extension
    std::string getFullPath();
    // returns the file extension, without preceding dot
    std::string getExtension();
    inline Status getStatus() 
    {
        std::scoped_lock lck(lock);
        return status;
    }
    inline void setStatus(Status status)
    {
        std::scoped_lock lck(lock);
        this->status = status;
    }
protected:
    std::mutex lock;
private:
    Status status;
    std::filesystem::path fullPath;
    std::filesystem::path parentDir;
    std::filesystem::path name;
    std::filesystem::path extension;
    uint32 byteSize;
    std::ifstream inStream;
    std::ofstream outStream;
};
DEFINE_REF(Asset);
} // namespace Seele