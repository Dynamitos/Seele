#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_REF(Material)
class MaterialAsset : public Asset
{
public:
    MaterialAsset();
    MaterialAsset(const std::string &directory, const std::string &name);
    MaterialAsset(const std::filesystem::path &fullPath);
    virtual ~MaterialAsset();
    virtual void beginFrame();
    virtual void endFrame();
    virtual void save() override;
    virtual void load() override;
    PMaterial getMaterial() const { return material; }
private:
    PMaterial material;
    friend class MaterialLoader;
};
DEFINE_REF(MaterialAsset)
} // namespace Seele
