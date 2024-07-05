#include "TextureLoader.h"
#include "Asset/AssetRegistry.h"
#include "Asset/TextureAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Vulkan/Enums.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-compare"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable"
#pragma GCC diagnostic ignored "-Wunused-variable"
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#pragma GCC diagnostic pop
#include "ktx.h"
#include <fstream>

using namespace Seele;

TextureLoader::TextureLoader(Gfx::PGraphics graphics) : graphics(graphics) {
    OTextureAsset placeholder = new TextureAsset();
    placeholderAsset = placeholder;
    import(
        TextureImportArgs{
            .filePath = std::filesystem::absolute("textures/placeholder.png"),
            .importPath = "",
        },
        placeholderAsset);
    AssetRegistry::get().assetRoot->textures[""] = std::move(placeholder);
}

TextureLoader::~TextureLoader() {}

void TextureLoader::importAsset(TextureImportArgs args) {
    std::string str = args.filePath.filename().string();
    auto pos = str.rfind(".");
    str.replace(str.begin() + pos, str.end(), "");

    OTextureAsset asset = new TextureAsset(args.importPath, str);
    PTextureAsset ref = asset;
    asset->setStatus(Asset::Status::Loading);
    AssetRegistry::get().registerTexture(std::move(asset));
    import(args, ref);
}

PTextureAsset TextureLoader::getPlaceholderTexture() { return placeholderAsset; }

#define KTX_ASSERT(x)                                                                                                                      \
    {                                                                                                                                      \
        auto error = x;                                                                                                                    \
        if (error != KTX_SUCCESS) {                                                                                                        \
            std::cout << ktxErrorString(error) << std::endl;                                                                               \
            abort();                                                                                                                       \
        }                                                                                                                                  \
    }

void TextureLoader::import(TextureImportArgs args, PTextureAsset textureAsset) {
    int totalWidth = 0, totalHeight = 0, n = 0;
    unsigned char* data = stbi_load(args.filePath.string().c_str(), &totalWidth, &totalHeight, &n, 0);
    ktxTexture2* kTexture = nullptr;
    VkFormat format;
    switch (n) {
    case 1:
        format = VK_FORMAT_R8_UNORM;
        break;
    case 2:
        format = VK_FORMAT_R8G8_UNORM;
        break;
    case 3:
        format = VK_FORMAT_R8G8B8_UNORM;
        break;
    case 4:
        format = VK_FORMAT_R8G8B8A8_UNORM;
        break;
    }
    ktxTextureCreateInfo createInfo = {
        .vkFormat = (uint32)format,
        .baseDepth = 1,
        .numLevels = 1,
        .numLayers = 1,
        .isArray = false,
        .generateMipmaps = false,
    };

    if (args.type == TextureImportType::TEXTURE_CUBEMAP) {
        uint32 faceWidth = totalWidth / 4;
        // uint32 faceHeight = totalHeight / 3;
        // Cube map
        createInfo.baseWidth = totalWidth / 4;
        createInfo.baseHeight = totalHeight / 3;
        createInfo.numFaces = 6;
        createInfo.numDimensions = 2;

        KTX_ASSERT(ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture));

        auto loadCubeFace = [n, &kTexture, faceWidth, totalWidth, &data](int xPos, int yPos, int faceName) {
            std::vector<unsigned char> vec(faceWidth * faceWidth * n);
            for (uint32 y = 0; y < faceWidth; ++y) {
                for (uint32 x = 0; x < faceWidth; ++x) {
                    int imgX = x + (xPos * faceWidth);
                    int imgY = y + (yPos * faceWidth);
                    std::memcpy(&vec[(x + (faceWidth * y)) * n], &data[(imgX + (totalWidth * imgY)) * n], n);
                }
            }
            ktxTexture_SetImageFromMemory(ktxTexture(kTexture), 0, 0, faceName, vec.data(), vec.size());
        };
        loadCubeFace(2, 1, 0); // +X
        loadCubeFace(0, 1, 1); // -X
        loadCubeFace(1, 0, 2); // +Y
        loadCubeFace(1, 2, 3); // -Y
        loadCubeFace(1, 1, 4); // +Z
        loadCubeFace(3, 1, 5); // -Z
    } else {
        createInfo.baseWidth = totalWidth;
        createInfo.baseHeight = totalHeight;
        createInfo.numFaces = 1;
        createInfo.numDimensions = 1 + (totalHeight > 1);

        ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);

        ktxTexture_SetImageFromMemory(ktxTexture(kTexture), 0, 0, 0, data, totalWidth * totalHeight * n * sizeof(unsigned char));
    }
    ktxBasisParams basisParams = {
        .structSize = sizeof(ktxBasisParams),
        .uastc = true,
        .threadCount = std::thread::hardware_concurrency() - 2,
        .uastcFlags = KTX_PACK_UASTC_LEVEL_SLOWER,
        .uastcRDO = true,
    };
    KTX_ASSERT(ktxTexture2_CompressBasisEx(kTexture, &basisParams));

    KTX_ASSERT(ktxTexture2_DeflateZstd(kTexture, 20));

    char writer[100];
    snprintf(writer, sizeof(writer), "%s version %s", "SeeleEngine", "0.0.1");
    ktxHashList_AddKVPair(&kTexture->kvDataHead, KTX_WRITER_KEY, (ktx_uint32_t)strlen(writer) + 1, writer);

    uint8* texData;
    size_t texSize;
    KTX_ASSERT(ktxTexture_WriteToMemory(ktxTexture(kTexture), &texData, &texSize));

    ktxTexture_WriteToNamedFile(ktxTexture(kTexture), args.filePath.filename().replace_extension(".ktx").generic_string().c_str());

    Array<uint8> serialized(texSize);
    std::memcpy(serialized.data(), texData, texSize);

    stbi_image_free(data);
    ktxTexture_Destroy(ktxTexture(kTexture));

    if (textureAsset->getName().empty()) {
        return;
    }

    textureAsset->setTexture(std::move(serialized));

    std::string path = (std::filesystem::path(args.importPath) / textureAsset->getName()).string().append(".asset");
    auto assetStream = AssetRegistry::createWriteStream(std::move(path), std::ios::binary);
    ArchiveBuffer buffer(graphics);
    // write identifier
    Serialization::save(buffer, TextureAsset::IDENTIFIER);
    // write name
    Serialization::save(buffer, textureAsset->getName());
    // write folder
    Serialization::save(buffer, textureAsset->getFolderPath());
    // write asset data
    textureAsset->save(buffer);

    buffer.writeToStream(assetStream);

    buffer.rewind();

    AssetRegistry::get().loadAsset(buffer);
}
