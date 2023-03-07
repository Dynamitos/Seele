#pragma once
#include "Asset.h"
#include "Material/MaterialInstance.h"

namespace Seele
{
class MaterialInstanceAsset : public Asset
{
public:
    static constexpr uint64 IDENTIFIER = 0x8;
    MaterialInstanceAsset();
    MaterialInstanceAsset(std::string_view folderPath, std::string_view name);
    virtual ~MaterialInstanceAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;
private:
    PMaterialInstance material;
};
DEFINE_REF(MaterialInstanceAsset)
} // namespace Seele