#include "TextureLoader.h"
#include "Asset/TextureAsset.h"
#include "Graphics/Graphics.h"
#include "Asset/AssetRegistry.h"
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
    //(std::filesystem::absolute("./textures/placeholder.ktx"));
    //placeholderAsset->load(graphics);
    //AssetRegistry::get().assetRoot.textures[""] = placeholderAsset;
    placeholderAsset = new TextureAsset();
    import(TextureImportArgs{
        .filePath = std::filesystem::absolute("./textures/placeholder.png"),
        .importPath = "",
        }, placeholderAsset);
    AssetRegistry::get().assetRoot->textures[""] = placeholderAsset;
}

TextureLoader::~TextureLoader()
{
}

void TextureLoader::importAsset(TextureImportArgs args)
{
    std::filesystem::path assetPath = args.filePath.filename();
    assetPath.replace_extension("asset");
    PTextureAsset asset = new TextureAsset(args.importPath, assetPath.stem().string());
    asset->setStatus(Asset::Status::Loading);
    asset->setTexture(placeholderAsset->getTexture());
    AssetRegistry::get().registerTexture(asset);
    import(args, asset);
}

PTextureAsset TextureLoader::getPlaceholderTexture() 
{
    return placeholderAsset;
}

void TextureLoader::import(TextureImportArgs args, PTextureAsset textureAsset)
{
    int totalWidth, totalHeight, n;
    unsigned char* data = stbi_load(args.filePath.string().c_str(), &totalWidth, &totalHeight, &n, 4);
    ktxTexture2* kTexture;
    ktxTextureCreateInfo createInfo;
    createInfo.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
    createInfo.baseDepth = 1;
    createInfo.numLevels = 1;
    createInfo.numLayers = 1;
    createInfo.isArray = false;
    createInfo.generateMipmaps = false;

    if (args.type == TextureImportType::TEXTURE_CUBEMAP)
    {
        uint32 faceWidth = totalWidth / 4;
        uint32 faceHeight = totalHeight / 3;
        // Cube map
        createInfo.baseWidth = totalWidth / 4;
        createInfo.baseHeight = totalHeight / 3;
        createInfo.numFaces = 6;
        createInfo.numDimensions = 2;

        ktxTexture2_Create(&createInfo,
            KTX_TEXTURE_CREATE_ALLOC_STORAGE,
            &kTexture);

        auto loadCubeFace = [&kTexture, &faceWidth, &totalWidth, &data](int xPos, int yPos, int faceName)
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
        loadCubeFace(2, 1, 0); // +X
        loadCubeFace(0, 1, 1); // -X
        loadCubeFace(1, 0, 2); // +Y
        loadCubeFace(1, 2, 3); // -Y
        loadCubeFace(1, 1, 4); // +Z
        loadCubeFace(3, 1, 5); // -Z
    }
    else
    {
        createInfo.baseWidth = totalWidth;
        createInfo.baseHeight = totalHeight;
        createInfo.numFaces = 1;
        createInfo.numDimensions = 1 + (totalHeight > 1);

        ktxTexture2_Create(&createInfo,
            KTX_TEXTURE_CREATE_ALLOC_STORAGE,
            &kTexture);

        ktxTexture_SetImageFromMemory(ktxTexture(kTexture),
            0, 0, 0, data, totalWidth * totalHeight * 4 * sizeof(unsigned char));
    }
    std::cout << "starting compression of " << textureAsset->getAssetIdentifier() << std::endl;
    ktxBasisParams params = {
        .structSize = sizeof(ktxBasisParams),
        .uastc = KTX_TRUE,
        .threadCount = std::thread::hardware_concurrency(),
        .qualityLevel = 0,
    };
    ktx_error_code_e error = ktxTexture2_CompressBasisEx(kTexture, &params);
    assert(error == KTX_SUCCESS);

    //error = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);
    //assert(error == KTX_SUCCESS);
    
    ktx_uint8_t* dest;
    ktx_size_t size;
    ktxTexture_WriteToMemory(ktxTexture(kTexture), &dest, &size);

    Array<uint8> memory(size);
    std::memcpy(memory.data(), dest, size);

    textureAsset->createFromMemory(std::move(memory), graphics);

    free(dest);
    stbi_image_free(data);
    ////co_return;
}