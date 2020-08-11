#include "TextureLoader.h"
#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "AssetRegistry.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace Seele;

TextureLoader::TextureLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    import("textures/placeholder.png");
    placeholderTexture = AssetRegistry::findTexture("textures/placeholder.png");
}

TextureLoader::~TextureLoader()
{
}

void TextureLoader::importAsset(const std::filesystem::path& filePath)
{
    futures.add(std::async(std::launch::async, &TextureLoader::import, this, filePath));
}

void TextureLoader::import(const std::filesystem::path& path)
{
    auto assetFileName = path;
    std::filesystem::path assetPath = AssetRegistry::get().rootFolder.append(assetFileName.replace_extension("asset").filename().string());
    PTextureAsset asset = new TextureAsset(assetPath);
    asset->setStatus(Asset::Status::Loading);
    int x, y, n;
    unsigned char* data = stbi_load(path.string().c_str(), &x, &y, &n, 4);
    TextureCreateInfo createInfo;
    createInfo.format = Gfx::SE_FORMAT_R8G8B8A8_UINT;
    createInfo.resourceData.data = data;
    createInfo.resourceData.size = x * y * 4 * sizeof(unsigned char);
    createInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    createInfo.width = x;
    createInfo.height = y;
    Gfx::PTexture2D texture = graphics->createTexture2D(createInfo);
    stbi_image_free(data);
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
    asset->setTexture(texture);
    asset->setStatus(Asset::Status::Ready);
    AssetRegistry::get().textures[path.string()] = asset;
}