#pragma once
#include "MaterialAsset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, DescriptorSet);
DECLARE_REF(Material);
class MaterialInstance : public MaterialAsset
{
public:
    MaterialInstance();
    MaterialInstance(const std::string& directory, const std::string& name);
    MaterialInstance(const std::filesystem::path& fullPath);
    ~MaterialInstance();
    virtual void save() override;
    virtual void load() override;
    virtual const Material* getRenderMaterial() const;
private:
    Material* baseMaterial;
};
DEFINE_REF(MaterialInstance);
} // namespace Seele