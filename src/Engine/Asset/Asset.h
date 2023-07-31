#pragma once
#include "MinimalEngine.h"
#include "Serialization/ArchiveBuffer.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Graphics)
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
    Asset(std::string_view folderPath, std::string_view name);
    virtual ~Asset();

    void updateByteSize();

    virtual void save(ArchiveBuffer& buffer) const = 0;
    virtual void load(ArchiveBuffer& buffer) = 0;
    
    bool isModified() const;
    // returns the assets name
    std::string getName() const;
    // returns the (virtual) folder path
    std::string getFolderPath() const;
    // returns the identifier with which it can be found from the asset registry
    std::string getAssetIdentifier() const;

    constexpr Status getStatus() 
    {
        return status;
    }
    constexpr void setStatus(Status _status)
    {
        status = _status;
    }
protected:
    std::string folderPath;
    std::string name;
    std::string assetId;
    Status status;
    uint64 byteSize;
};
DEFINE_REF(Asset)
} // namespace Seele