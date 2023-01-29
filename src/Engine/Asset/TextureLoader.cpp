#include "TextureLoader.h"
#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "AssetRegistry.h"
#include "Graphics/Vulkan/VulkanGraphicsEnums.h"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include "ktx.h"

using namespace Seele;

TextureLoader::TextureLoader(Gfx::PGraphics graphics)
    : graphics(graphics)
{
    placeholderAsset = new TextureAsset(std::filesystem::absolute("./textures/placeholder.ktx"));
    placeholderAsset->load(graphics);
    AssetRegistry::get().assetRoot.textures[""] = placeholderAsset;
}

TextureLoader::~TextureLoader()
{
}

void TextureLoader::importAsset(const std::filesystem::path& path, const std::string& importPath)
{
    std::filesystem::path assetPath = path.filename();
    assetPath.replace_extension("asset");
    PTextureAsset asset = new TextureAsset(assetPath.generic_string());
    asset->setStatus(Asset::Status::Loading);
    asset->setTexture(placeholderAsset->getTexture());
    AssetRegistry::get().registerTexture(asset, importPath);
    import(path, asset);
}

PTextureAsset TextureLoader::getPlaceholderTexture() 
{
    return placeholderAsset;
}

void TextureLoader::import(std::filesystem::path path, PTextureAsset textureAsset)
{
    int totalWidth, totalHeight, n;
    unsigned char* data = stbi_load(path.string().c_str(), &totalWidth, &totalHeight, &n, 4);
    ktxTexture2* kTexture;
    ktxTextureCreateInfo createInfo;

    if (totalWidth / 4 == totalHeight / 3)
    {
        uint32 faceWidth = totalWidth / 4;
        uint32 faceHeight = totalHeight / 3;
        // Cube map
        createInfo.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
        createInfo.baseWidth = totalWidth / 4;
        createInfo.baseHeight = totalHeight / 3;
        createInfo.baseDepth = 1;
        createInfo.numFaces = 6;
        createInfo.numDimensions = 3;
        createInfo.numLevels = 1;
        createInfo.numLayers = 1;
        createInfo.isArray = false;
        createInfo.generateMipmaps = true;

        ktxTexture2_Create(&createInfo,
            KTX_TEXTURE_CREATE_ALLOC_STORAGE,
            &kTexture);

        auto loadCubeFace = [kTexture, faceWidth, totalWidth, data](int xPos, int yPos, int faceName)
        {
            std::vector<unsigned char> vec(faceWidth * faceWidth * 4);
            for (int y = 0; y < faceWidth; ++y)
            {
                for (int x = 0; x < faceWidth; ++x)
                {
                    int imgX = x + (xPos * faceWidth);
                    int imgY = y + (yPos * faceWidth);
                    std::memcpy(&vec[(x + (faceWidth * y)) * 4], &data[(imgX + (totalWidth * imgY)) * 4], 4);
                }
            }
            ktxTexture_SetImageFromMemory(ktxTexture(kTexture),
                0, 0, faceName, vec.data(), vec.size());
        };
        loadCubeFace(1, 0, 0);
        loadCubeFace(0, 1, 1);
        loadCubeFace(1, 1, 2);
        loadCubeFace(2, 1, 3);
        loadCubeFace(3, 1, 4);
        loadCubeFace(1, 2, 5);
    }
    else
    {
        createInfo.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
        createInfo.baseWidth = totalWidth;
        createInfo.baseHeight = totalHeight;
        createInfo.baseDepth = 1;
        createInfo.numDimensions = 1 + (totalHeight > 1);
        createInfo.numLevels = 1;
        createInfo.numLayers = 1;
        createInfo.numFaces = 1;
        createInfo.isArray = false;
        createInfo.generateMipmaps = true;

        ktxTexture2_Create(&createInfo,
            KTX_TEXTURE_CREATE_ALLOC_STORAGE,
            &kTexture);

        ktxTexture_SetImageFromMemory(ktxTexture(kTexture),
            0, 0, 0, data, totalWidth * totalHeight * 4 * sizeof(unsigned char));
    }

    stbi_image_free(data);

    ktxTexture_WriteToNamedFile(ktxTexture(kTexture), textureAsset->getFullPath().c_str());
    ktxTexture_Destroy(ktxTexture(kTexture));

    textureAsset->load(graphics);
    textureAsset->setStatus(Asset::Status::Ready);
    ////co_return;
}