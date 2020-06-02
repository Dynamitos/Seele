#include "TextureLoader.h"
#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "AssetRegistry.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Seele;

TextureLoader::TextureLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    import("./textures/placeholder.png");
    placeholderTexture = AssetRegistry::findTexture("./textures/placeholder.png");
}

TextureLoader::~TextureLoader()
{
}

void TextureLoader::importAsset(const std::string& filePath)
{
    futures.add(std::async(std::launch::async, &TextureLoader::import, this, filePath));
}

void TextureLoader::import(const std::string& path)
{
    PTextureAsset asset = new TextureAsset(path);
    AssetRegistry::get().textures[path] = asset;
    asset->setStatus(Asset::Status::Loading); 
    int x, y, n;
    const std::string fullPath = std::string(asset->getFullPath());
    unsigned char* data = stbi_load(fullPath.c_str(), &x, &y, &n, 0);
    TextureCreateInfo createInfo;
    createInfo.resourceData.data = data;
    createInfo.resourceData.size = x * y * n * sizeof(unsigned char);
    createInfo.width = x;
    createInfo.height = y;
    switch (n)
    {
    case 1:
        createInfo.format = Gfx::SE_FORMAT_R8_UINT;
        break;
    case 2:
        createInfo.format = Gfx::SE_FORMAT_R8G8_UINT;
        break;
    case 3:
        createInfo.format = Gfx::SE_FORMAT_R8G8B8_UINT;
        break;
    case 4:
        createInfo.format = Gfx::SE_FORMAT_R8G8B8A8_UINT;
        break;
    default:
        break;
    }
    createInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    Gfx::PTexture2D texture = graphics->createTexture2D(createInfo);
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
    asset->setTexture(texture);
    asset->setStatus(Asset::Status::Ready);
}