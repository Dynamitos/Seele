#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "Graphics/Vulkan/Enums.h"
#include "Graphics/Texture.h"
#include "ktx.h"

using namespace Seele;

#define KTX_CHECK(x) { ktx_error_code_e err = x; assert(err == KTX_SUCCESS); }

TextureAsset::TextureAsset()
{
}

TextureAsset::TextureAsset(std::string_view folderPath, std::string_view name)
    : Asset(folderPath, name)
{
}

TextureAsset::~TextureAsset() 
{
    ktxTexture_Destroy(ktxTexture(ktxHandle));
}

void TextureAsset::save(ArchiveBuffer& buffer) const
{
    char writer[100];
    snprintf(writer, sizeof(writer), "%s version %s", "SeeleEngine", "0.0.1");
    ktxHashList_AddKVPair(&ktxHandle->kvDataHead, KTX_WRITER_KEY,
        (ktx_uint32_t)strlen(writer) + 1,
        writer);

    ktx_uint8_t* texData;
    ktx_size_t texSize;
    KTX_CHECK(ktxTexture_WriteToMemory(ktxTexture(ktxHandle), &texData, &texSize));

    Array<uint8> rawData(texSize);
    std::memcpy(rawData.data(), texData, texSize);
    Serialization::save(buffer, rawData);
    free(texData);
}

void TextureAsset::load(ArchiveBuffer& buffer) 
{
    Gfx::PGraphics graphics = buffer.getGraphics();
    Array<uint8> rawData;
    Serialization::load(buffer, rawData);
    KTX_CHECK(ktxTexture_CreateFromMemory(rawData.data(),
        rawData.size(),
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        (ktxTexture**)&ktxHandle));

    //ktx_error_code_e e = ktxTexture2_TranscodeBasis(ktxHandle, KTX_TTF_BC7_RGBA, 0);
    //assert(e == ktx_error_code_e::KTX_SUCCESS);

    TextureCreateInfo createInfo = {
            .sourceData = {
                .size = ktxTexture_GetDataSize(ktxTexture(ktxHandle)),
                .data = ktxTexture_GetData(ktxTexture(ktxHandle)),
                .owner = Gfx::QueueType::TRANSFER,
            },
            .format = (Gfx::SeFormat)ktxHandle->vkFormat,
            .width = ktxHandle->baseWidth,
            .height = ktxHandle->baseHeight,
            .depth = ktxHandle->baseDepth,
            .mipLevels = ktxHandle->numLevels,
            .layers = ktxHandle->numFaces,
            .elements = ktxHandle->numLayers,
            .usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
    };
    if (ktxHandle->isCubemap)
    {
        texture = graphics->createTextureCube(createInfo);
    }
    else if (ktxHandle->isArray)
    {
        texture = graphics->createTexture3D(createInfo);
    }
    else
    {
        texture = graphics->createTexture2D(createInfo);
    }
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
}

void TextureAsset::setTexture(Gfx::OTexture _texture)
{
    texture = std::move(_texture);    
}

uint32 TextureAsset::getWidth()
{
    return texture->getWidth();
}

uint32 TextureAsset::getHeight()
{
    return texture->getHeight();
}
