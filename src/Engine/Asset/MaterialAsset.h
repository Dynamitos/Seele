#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_REF(Material)
class MaterialAsset : public Asset
{
public:
    static constexpr uint64 IDENTIFIER = 0x4;
    MaterialAsset();
    MaterialAsset(std::string_view folderPath, std::string_view name);
    virtual ~MaterialAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
    PMaterial getMaterial() const { return material; }
private:
    PMaterial material;
    friend class MaterialLoader;
};
DEFINE_REF(MaterialAsset)
} // namespace Seele
