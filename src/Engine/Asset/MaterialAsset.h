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
    virtual void save(Gfx::PGraphics graphics) override;
    virtual void load(Gfx::PGraphics graphics) override;
    PMaterial getMaterial() const { return material; }
private:
    PMaterial material;
    friend class MaterialLoader;
};
DEFINE_REF(MaterialAsset)
} // namespace Seele
