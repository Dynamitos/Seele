#pragma once
#include "Containers/List.h"
#include "MinimalEngine.h"
#include <filesystem>


namespace Seele {
DECLARE_REF(MaterialAsset)
DECLARE_NAME_REF(Gfx, Graphics)
struct MaterialImportArgs {
    std::filesystem::path filePath;
    std::string importPath;
};
class MaterialLoader {
  public:
    MaterialLoader(Gfx::PGraphics graphic);
    ~MaterialLoader();
    void importAsset(MaterialImportArgs args);
    PMaterialAsset getPlaceHolderMaterial();

  private:
    void import(MaterialImportArgs args, PMaterialAsset asset);
    Gfx::PGraphics graphics;
    PMaterialAsset placeholderMaterial;
};
DEFINE_REF(MaterialLoader)
} // namespace Seele