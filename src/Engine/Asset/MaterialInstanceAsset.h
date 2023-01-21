#pragma once
#include "Asset.h"
#include "Material/MaterialInstance.h"

namespace Seele
{
class MaterialInstanceAsset : public Asset
{
public:
    MaterialInstanceAsset();
    MaterialInstanceAsset(const std::string &directory, const std::string &name);
    MaterialInstanceAsset(const std::filesystem::path &fullPath);
    virtual ~MaterialInstanceAsset();
    virtual void beginFrame();
    virtual void endFrame();
    virtual void save() override;
    virtual void load() override;
private:
    PMaterialInstance material;
};
DEFINE_REF(MaterialInstanceAsset)
} // namespace Seele
