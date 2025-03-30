#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"
#include "Graphics/Vulkan/Enums.h"
#include "Window/WindowManager.h"
#include "ktx.h"

using namespace Seele;

#define KTX_ASSERT(x)                                                                                                                      \
    {                                                                                                                                      \
        auto error = x;                                                                                                                    \
        if (error != KTX_SUCCESS) {                                                                                                        \
            std::cout << ktxErrorString(error) << std::endl;                                                                               \
            abort();                                                                                                                       \
        }                                                                                                                                  \
    }

TextureAsset::TextureAsset() {}

TextureAsset::TextureAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {}

TextureAsset::~TextureAsset() {}

void TextureAsset::save(ArchiveBuffer& buffer) const { 
    Serialization::save(buffer, ktxData);
}

void TextureAsset::load(ArchiveBuffer& buffer) {
    ktxTexture2* ktxHandle;
    Serialization::load(buffer, ktxData);
    KTX_ASSERT(
        ktxTexture_CreateFromMemory(ktxData.data(), ktxData.size(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, (ktxTexture**)&ktxHandle));

    ktxTexture2_TranscodeBasis(ktxHandle, KTX_TTF_BC7_RGBA, 0);

    Gfx::PGraphics graphics = buffer.getGraphics();
    TextureCreateInfo createInfo = {
        .sourceData =
            {
                .size = ktxTexture_GetDataSize(ktxTexture(ktxHandle)),
                .data = ktxTexture_GetData(ktxTexture(ktxHandle)),
                .owner = Gfx::QueueType::GRAPHICS,
            },
        .format = (Gfx::SeFormat)ktxHandle->vkFormat,
        .width = ktxHandle->baseWidth,
        .height = ktxHandle->baseHeight,
        .depth = ktxHandle->baseDepth,
        .layers = ktxHandle->numFaces,
        .elements = ktxHandle->numLayers,
        .useMip = true,
        .usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
        .name = name,
    };
    if (ktxHandle->isCubemap) {
        texture = graphics->createTextureCube(createInfo);
    } else if (ktxHandle->isArray) {
        texture = graphics->createTexture3D(createInfo);
    } else {
        texture = graphics->createTexture2D(createInfo);
    }

    ktxTexture_Destroy(ktxTexture(ktxHandle));

    byteSize = sizeof(TextureAsset) + ktxData.size();
}

uint32 TextureAsset::getWidth() { return texture->getWidth(); }

uint32 TextureAsset::getHeight() { return texture->getHeight(); }
