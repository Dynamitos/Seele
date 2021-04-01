#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <thread>
#include <future>
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
    void importAsset(const std::filesystem::path& filePath);
    PTextureAsset getPlaceholderTexture();
private:
    Gfx::PTexture2D import(const std::filesystem::path& path);
    Gfx::PGraphics graphics;
    List<std::future<void>> futures;
    Gfx::PTexture2D placeholderTexture;
    PTextureAsset placeholderAsset;
};
DEFINE_REF(TextureLoader)
} // namespace Seele