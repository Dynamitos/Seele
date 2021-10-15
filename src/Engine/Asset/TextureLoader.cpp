#include "TextureLoader.h"
#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "AssetRegistry.h"
#include "Graphics/Vulkan/VulkanGraphicsEnums.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

using namespace Seele;

TextureLoader::TextureLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    placeholderAsset = new TextureAsset(std::filesystem::absolute("./textures/placeholder.ktx"));
    placeholderAsset->load();
    AssetRegistry::get().textures[""] = placeholderAsset;
}

TextureLoader::~TextureLoader()
{
}

void TextureLoader::importAsset(const std::filesystem::path& filePath)
{
    auto assetFileName = filePath;
    PTextureAsset asset = new TextureAsset(assetFileName.replace_extension("asset").filename().generic_string());
    asset->setStatus(Asset::Status::Loading);
    asset->setTexture(placeholderAsset->getTexture());
    std::cout << "Loading texture " << asset->getFileName() << std::endl;
    AssetRegistry::get().registerTexture(asset);
    futures.add(std::async(std::launch::async, [this, filePath, asset] () mutable {
        using namespace std::chrono_literals;
        //std::this_thread::sleep_for(5s);
        import(filePath, asset);
        asset->load();
        asset->setStatus(Asset::Status::Ready);
    }));
}

PTextureAsset TextureLoader::getPlaceholderTexture() 
{
    return placeholderAsset;
}

void TextureLoader::import(const std::filesystem::path& path, PTextureAsset textureAsset)
{
    int x, y, n;
    unsigned char* data = stbi_load(path.string().c_str(), &x, &y, &n, 4);
    ktxTexture2* kTexture;
    ktxTextureCreateInfo createInfo;

    createInfo.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
    createInfo.baseWidth = x;
    createInfo.baseHeight = y;
    createInfo.baseDepth = 1;
    createInfo.numDimensions = 1 + (y > 1);
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.numFaces = 1;
    createInfo.isArray = false;
    createInfo.generateMipmaps = true;

    ktxTexture2_Create(&createInfo,
        KTX_TEXTURE_CREATE_ALLOC_STORAGE,
        &kTexture);

    ktxTexture_SetImageFromMemory(ktxTexture(kTexture),
        0, 0, 0, data, x * y * 4 * sizeof(unsigned char));

    stbi_image_free(data);

    ktxTexture_WriteToNamedFile(ktxTexture(kTexture), textureAsset->getFullPath().c_str());
    ktxTexture_Destroy(ktxTexture(kTexture));
}