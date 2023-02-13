#include "TextureAsset.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "Graphics/Vulkan/VulkanGraphicsEnums.h"
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

void TextureAsset::save(ArchiveBuffer& buffer) const
{
    Array<uint8> textureData;
    ktxTexture2* kTexture;
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
            const_cast<Gfx::Texture*>(texture.getHandle())->download(0, depth, face, textureData);
            KTX_CHECK(ktxTexture_SetImageFromMemory(ktxTexture(kTexture), 0, depth, face, textureData.data(), textureData.size()));
        }
    }
    char writer[100];
    snprintf(writer, sizeof(writer), "%s version %s", "SeeleEngine", "0.0.1");
    ktxHashList_AddKVPair(&kTexture->kvDataHead, KTX_WRITER_KEY,
        (ktx_uint32_t)strlen(writer) + 1,
        writer);

    //ktx_error_code_e error = ktxTexture2_CompressBasis(kTexture, 0);

    ktx_uint8_t* texData;
    ktx_size_t texSize;
    KTX_CHECK(ktxTexture_WriteToMemory(ktxTexture(kTexture), &texData, &texSize));

    Array<uint8> rawData(texSize);
    std::memcpy(rawData.data(), texData, texSize);
    free(texData);
    
    Serialization::save(buffer, rawData);
}

void TextureAsset::load(ArchiveBuffer& buffer) 
{
    setStatus(Status::Loading);

    Array<uint8> rawData;
    Serialization::load(buffer, rawData);

    ktxTexture2* kTexture;
    std::cout << "Loading texture " << name << std::endl;
    KTX_CHECK(ktxTexture_CreateFromMemory(rawData.data(),
        rawData.size(),
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, 
        (ktxTexture**) & kTexture));

    //ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

    TextureCreateInfo createInfo = {
            .resourceData = {
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
    Gfx::PTexture tex;
    if (kTexture->isCubemap)
    {
        tex = buffer.getGraphics()->createTextureCube(createInfo);
    }
    else if (kTexture->isArray)
    {
        tex = buffer.getGraphics()->createTexture3D(createInfo);
    }
    else
    {
        tex = buffer.getGraphics()->createTexture2D(createInfo);
    }
    tex->transferOwnership(Gfx::QueueType::GRAPHICS);
    setTexture(tex);
    ktxTexture_Destroy(ktxTexture(kTexture));
    setStatus(Asset::Status::Ready);
}