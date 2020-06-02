#pragma once
#include "Asset/Asset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, DescriptorSet);
DECLARE_REF(Material);
class MaterialInstance : public Asset
{
public:
    MaterialInstance();
    MaterialInstance(const std::string& directory, const std::string& name);
    MaterialInstance(const std::string& fullPath);
    ~MaterialInstance();
    PMaterial getBaseMaterial() const;
    Gfx::PDescriptorSet getDescriptor();
private:
    PMaterial baseMaterial;
};
DEFINE_REF(MaterialInstance);
} // namespace Seele