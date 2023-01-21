#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <filesystem>

namespace Seele
{
DECLARE_REF(TextureAsset)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, Texture2D)
class TextureLoader
{
public:
    TextureLoader(Gfx::PGraphics graphic);
    ~TextureLoader();
    void importAsset(const std::filesystem::path& filePath, const std::string& importPath);
    PTextureAsset getPlaceholderTexture();
private:
    void import(std::filesystem::path path, PTextureAsset asset);
    Gfx::PGraphics graphics;
    PTextureAsset placeholderAsset;
};
DEFINE_REF(TextureLoader)
} // namespace Seele