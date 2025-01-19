#pragma once
#include "Containers/List.h"
#include "MinimalEngine.h"
#include <filesystem>

namespace Seele {
DECLARE_REF(SVGAsset)
DECLARE_NAME_REF(Gfx, Graphics)
struct SVGImportArgs {
    std::filesystem::path filePath;
    std::string importPath;
};
class SVGLoader {
  public:
    SVGLoader(Gfx::PGraphics graphic);
    ~SVGLoader();
    void importAsset(SVGImportArgs args);

  private:
    void import(SVGImportArgs args, PSVGAsset asset);
    Gfx::PGraphics graphics;
};
DECLARE_REF(SVGLoader)
} // namespace Seele