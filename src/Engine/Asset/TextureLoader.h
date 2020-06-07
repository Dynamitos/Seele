#pragma once
#include "MinimalEngine.h"
#include "Containers/List.h"
#include <thread>
#include <future>

namespace Seele
{
DECLARE_REF(TextureAsset);
DECLARE_NAME_REF(Gfx, Graphics);
class TextureLoader
{
public:
    TextureLoader(Gfx::PGraphics graphic);
    ~TextureLoader();
    void importAsset(const std::filesystem::path& filePath);
private:
    void import(const std::filesystem::path& path);
    Gfx::PGraphics graphics;
    List<std::future<void>> futures;
    PTextureAsset placeholderTexture;
};
DEFINE_REF(TextureLoader);
} // namespace Seele