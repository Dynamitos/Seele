#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_REF(Material)
DECLARE_REF(MaterialInstanceAsset)
struct InstantiationParameter
{
    std::string name;
    std::string folderPath;
};
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
    OMaterialInstanceAsset instantiate(const InstantiationParameter& params);
private:
    OMaterial material;
    friend class MaterialLoader;
};
DEFINE_REF(MaterialAsset)
} // namespace Seele
