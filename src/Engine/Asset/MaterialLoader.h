#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <filesystem>

namespace Seele
{
DECLARE_REF(MaterialAsset)
DECLARE_NAME_REF(Gfx, Graphics)
class MaterialLoader
{
public:
    MaterialLoader(Gfx::PGraphics graphic);
    ~MaterialLoader();
    void importAsset(const std::filesystem::path& name, const std::string& importPath);
    PMaterialAsset getPlaceHolderMaterial();
private:
    void import(std::filesystem::path filePath, PMaterialAsset asset);
    Gfx::PGraphics graphics;
    PMaterialAsset placeholderMaterial;
};
DEFINE_REF(MaterialLoader)
} // namespace Seele