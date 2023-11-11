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
    
}

void TextureAsset::save(ArchiveBuffer&) const
{
    /*ktxTexture2* kTexture;
    ktxTextureCreateInfo createInfo = {
        .vkFormat = (uint32_t)texture->getFormat(),
        .baseWidth = texture->getSizeX(),
        .baseHeight = texture->getSizeY(),
        .baseDepth = texture->getSizeZ(),
        .numDimensions = texture->getSizeZ() > 1 ? 3u : texture->getSizeY() > 1 ? 2u : 1u,
        .numLevels = texture->getMipLevels(),
        .numLayers = 1,
        .numFaces = texture->getNumFaces(),
        .isArray = false,
        .generateMipmaps = false,
    };
    KTX_CHECK(ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture));

    for (uint32 depth = 0; depth < texture->getSizeZ(); ++depth)
    {
        for (uint32 face = 0; face < texture->getNumFaces(); ++face)
        {
            // technically, downloading cant be const, because we have to allocate temporary buffers and change layouts
            // but practically the texture stays the same
            Array<uint8> faceData;
            const_cast<Gfx::Texture*>(texture.getHandle())->download(0, depth, face, faceData);
            KTX_CHECK(ktxTexture_SetImageFromMemory(ktxTexture(kTexture), 0, depth, face, faceData.data(), faceData.size()));
        }
    }
    char writer[100];
    snprintf(writer, sizeof(writer), "%s version %s", "SeeleEngine", "0.0.1");
    ktxHashList_AddKVPair(&kTexture->kvDataHead, KTX_WRITER_KEY,
        (ktx_uint32_t)strlen(writer) + 1,
        writer);

    KTX_CHECK(ktxTexture2_CompressAstc(kTexture, 0));

    ktx_uint8_t* texData;
    ktx_size_t texSize;
    KTX_CHECK(ktxTexture_WriteToMemory(ktxTexture(kTexture), &texData, &texSize));

    Array<uint8> rawData(texSize);
    std::memcpy(rawData.data(), texData, texSize);
    Serialization::save(buffer, rawData);
    free(texData);*/
    assert(false); // TODO
}

void TextureAsset::load(ArchiveBuffer& buffer) 
{
    Gfx::PGraphics graphics = buffer.getGraphics();
    Serialization::load(buffer, rawPixels);
    Array<uint8> rawData;
    Serialization::load(buffer, rawData);
    ktxTexture2* kTexture;
    KTX_CHECK(ktxTexture_CreateFromMemory(rawData.data(),
        rawData.size(),
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
        (ktxTexture**)&kTexture));

    ktx_error_code_e e = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);
    assert(e == ktx_error_code_e::KTX_SUCCESS);

    TextureCreateInfo createInfo = {
            .sourceData = {
                .size = ktxTexture_GetDataSize(ktxTexture(kTexture)),
                .data = ktxTexture_GetData(ktxTexture(kTexture)),
                .owner = Gfx::QueueType::DEDICATED_TRANSFER,
            },
            .width = kTexture->baseWidth,
            .height = kTexture->baseHeight,
            .depth = kTexture->baseDepth,
            .bArray = kTexture->isArray,
            .arrayLayers = kTexture->isArray ? kTexture->numLayers : kTexture->numFaces,
            .mipLevels = kTexture->numLevels,
            .format = Vulkan::cast((VkFormat)kTexture->vkFormat),
            .usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
    };
    if (kTexture->isCubemap)
    {
        texture = graphics->createTextureCube(createInfo);
    }
    else if (kTexture->isArray)
    {
        texture = graphics->createTexture3D(createInfo);
    }
    else
    {
        texture = graphics->createTexture2D(createInfo);
    }
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
    ktxTexture_Destroy(ktxTexture(kTexture));
}

void TextureAsset::setTexture(Gfx::OTexture _texture)
{
    texture = std::move(_texture);    
}

uint32 TextureAsset::getWidth()
{
    return texture->getSizeX();
}

uint32 TextureAsset::getHeight()
{
    return texture->getSizeY();
}
