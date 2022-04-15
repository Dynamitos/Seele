#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <filesystem>

namespace Seele
{
DECLARE_REF(FontAsset)
DECLARE_NAME_REF(Gfx, Graphics)
class FontLoader
{
public:
    FontLoader(Gfx::PGraphics graphic);
    ~FontLoader();
    void importAsset(const std::filesystem::path& filePath);
private:
    void import(std::filesystem::path path, PFontAsset asset);
    Gfx::PGraphics graphics;
};
DEFINE_REF(FontLoader)
} // namespace Seele