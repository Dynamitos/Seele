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
    Asset(const std::string& directory, const std::string& name);
    Asset(const std::filesystem::path& path);
    virtual ~Asset();

    virtual void save() = 0;
    virtual void load() = 0;
    
    // returns the name of the file, without extension
    std::string getFileName() const;
    // returns the full absolute path, from root to extension
    std::string getFullPath() const;
    // returns the file extension, without preceding dot
    std::string getExtension() const;
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
    std::ifstream getReadStream() const;
    std::ofstream getWriteStream() const;
private:
    // Path relative to the project root
    std::filesystem::path fullPath;
    std::filesystem::path name;
    std::filesystem::path parentDir;
    std::filesystem::path extension;
    Status status;
    uint32 byteSize;
};
DEFINE_REF(Asset)
} // namespace Seele