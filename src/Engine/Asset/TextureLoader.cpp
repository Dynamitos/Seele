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
    placeholderAsset = new TextureAsset();
    placeholderTexture = import("./textures/placeholder.png");
    placeholderAsset->setTexture(placeholderTexture);
    placeholderAsset->setStatus(Asset::Status::Ready);
}

TextureLoader::~TextureLoader()
{
}

void TextureLoader::importAsset(const std::filesystem::path& filePath)
{
    auto assetFileName = filePath;
    PTextureAsset asset = new TextureAsset(assetFileName.replace_extension("asset").filename().generic_string());
    asset->setStatus(Asset::Status::Loading);
    asset->setTexture(placeholderTexture);
    std::cout << "Loading texture, placeholder" << std::endl;
    AssetRegistry::get().textures[asset->getFileName()] = asset;
    futures.add(std::async(std::launch::async, [this, filePath, asset] () mutable {
        Gfx::PTexture2D texture = import(filePath);
        asset->setTexture(texture);
        asset->setStatus(Asset::Status::Ready);
        std::cout << "Finished loading texture" << std::endl;
    }));
}

PTextureAsset TextureLoader::getPlaceholderTexture() 
{
    return placeholderAsset;
}

Gfx::PTexture2D TextureLoader::import(const std::filesystem::path& path)
{
    int x, y, n;
    unsigned char* data = stbi_load(path.string().c_str(), &x, &y, &n, 4);
    
    TextureCreateInfo createInfo;
    createInfo.format = Gfx::SE_FORMAT_R8G8B8A8_SRGB;
    createInfo.resourceData.data = data;
    createInfo.resourceData.size = x * y * 4 * sizeof(unsigned char);
    createInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    createInfo.width = x;
    createInfo.height = y;
    Gfx::PTexture2D texture = graphics->createTexture2D(createInfo);
    stbi_image_free(data);
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
    return texture;
}