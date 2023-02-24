#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <filesystem>

namespace Seele
{
DECLARE_REF(FontAsset)
DECLARE_NAME_REF(Gfx, Graphics)
struct FontImportArgs
{
    std::filesystem::path filePath;
    std::string importPath;
};
class FontLoader
{
public:
    FontLoader(Gfx::PGraphics graphic);
    ~FontLoader();
    void importAsset(FontImportArgs args);
private:
    void import(FontImportArgs args, PFontAsset asset);
    Gfx::PGraphics graphics;
};
DEFINE_REF(FontLoader)
} // namespace Seele