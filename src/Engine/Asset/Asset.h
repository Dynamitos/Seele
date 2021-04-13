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
        std::unique_lock lck(lock);
        return status;
    }
    inline void setStatus(Status status)
    {
        std::unique_lock lck(lock);
        this->status = status;
        if(status == Status::Ready)
        {
            readyCV.notify_all();
        }
    }
protected:
    inline void waitReady()
    {
        std::unique_lock lck(lock);
        if(status != Status::Ready)
        {
            std::cout << "Asset " << name.generic_string() << " not ready yet, waiting" << std::endl;
            readyCV.wait(lck);
            std::cout << "Asset " << name.generic_string() << " now ready, continuing" << std::endl;
        }
    }
    std::mutex lock;
    std::condition_variable readyCV;
    std::ifstream& getReadStream();
    std::ofstream& getWriteStream();
private:
    // Path relative to the project root
    std::filesystem::path fullPath;
    std::filesystem::path name;
    std::filesystem::path parentDir;
    std::filesystem::path extension;
    Status status;
    uint32 byteSize;
    std::ifstream inStream;
    std::ofstream outStream;
};
DEFINE_REF(Asset)
} // namespace Seele