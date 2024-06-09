#pragma once
#include "Containers/List.h"
#include "Graphics/Enums.h"
#include "MinimalEngine.h"
#include <filesystem>


namespace Seele {
DECLARE_REF(TextureAsset)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, Texture2D)
enum class TextureImportType {
    TEXTURE_2D,
    TEXTURE_NORMAL,
    TEXTURE_CUBEMAP,
};
struct TextureImportArgs {
    std::filesystem::path filePath;
    std::string importPath;
    TextureImportType type = TextureImportType::TEXTURE_2D;
    Gfx::SeImageUsageFlagBits usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT;
    uint32 numChannels = 4;
};
class TextureLoader {
  public:
    TextureLoader(Gfx::PGraphics graphic);
    ~TextureLoader();
    void importAsset(TextureImportArgs args);
    PTextureAsset getPlaceholderTexture();

  private:
    void import(TextureImportArgs args, PTextureAsset asset);
    Gfx::PGraphics graphics;
    PTextureAsset placeholderAsset;
};
DEFINE_REF(TextureLoader)
} // namespace Seele