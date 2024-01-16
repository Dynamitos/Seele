#include "FontAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Vulkan/Enums.h"
#include "Graphics/Texture.h"
#include <ktx.h>

using namespace Seele;

FontAsset::FontAsset()
{
}

FontAsset::FontAsset(std::string_view folderPath, std::string_view name)
    : Asset(folderPath, name)
{
}

FontAsset::~FontAsset()
{
    
}

void FontAsset::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, glyphs);
    Serialization::save(buffer, usedTextures.size());
    for (uint32 x = 0; x < usedTextures.size(); ++x)
    {
        Array<uint8> textureData;
        ktxTexture2* kTexture;
        ktxTextureCreateInfo createInfo = {
            .glInternalformat = 0,
            .vkFormat = (uint32_t)usedTextures[x]->getFormat(),
            .pDfd = nullptr,
            .baseWidth = usedTextures[x]->getWidth(),
            .baseHeight = usedTextures[x]->getHeight(),
            .baseDepth = usedTextures[x]->getDepth(),
            .numDimensions = usedTextures[x]->getDepth() > 1 ? 3u : usedTextures[x]->getHeight() > 1 ? 2u : 1u,
            .numLevels = usedTextures[x]->getMipLevels(),
            .numLayers = usedTextures[x]->getDepth(),
            .numFaces = usedTextures[x]->getNumFaces(),
            .isArray = false,
            .generateMipmaps = false,
        };
        ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);

        for (uint32 depth = 0; depth < usedTextures[x]->getDepth(); ++depth)
        {
            for (uint32 face = 0; face < usedTextures[x]->getNumFaces(); ++face)
            {
                // technically, downloading cant be const, because we have to allocate temporary buffers and change layouts
                // but practically the texture stays the same
                const_cast<Gfx::Texture2D*>(*(usedTextures[x]))->download(0, depth, face, textureData);
                ktxTexture_SetImageFromMemory(ktxTexture(kTexture), 0, depth, face, textureData.data(), textureData.size());
            }
        }
        char writer[100];
        snprintf(writer, sizeof(writer), "%s version %s", "SeeleEngine", "0.0.1");
        ktxHashList_AddKVPair(&kTexture->kvDataHead, KTX_WRITER_KEY,
            (ktx_uint32_t)strlen(writer) + 1,
            writer);

        ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_ASTC_4x4_RGBA, 0);

        ktx_uint8_t* texData;
        ktx_size_t texSize;
        ktxTexture_WriteToMemory(ktxTexture(kTexture), &texData, &texSize);

        Array<uint8> rawData(texSize);
        std::memcpy(rawData.data(), texData, texSize);

        free(texData);

        Serialization::save(buffer, rawData);
    }
}


void FontAsset::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, glyphs);
    size_t numTextures;
    Serialization::load(buffer, numTextures);
    for (uint64 x = 0; x < numTextures; ++x)
    {
        Array<uint8> rawTex;
        Serialization::load(buffer, rawTex);

        ktxTexture2* kTexture;
        ktxTexture2_CreateFromMemory(rawTex.data(),
            rawTex.size(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &kTexture);

        ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

        TextureCreateInfo createInfo = {
            .sourceData = {
                .size = ktxTexture_GetDataSize(ktxTexture(kTexture)),
                .data = ktxTexture_GetData(ktxTexture(kTexture)),
                .owner = Gfx::QueueType::GRAPHICS,
            },
            .format = Vulkan::cast((VkFormat)kTexture->vkFormat),
            .width = kTexture->baseWidth,
            .height = kTexture->baseHeight,
            .depth = kTexture->baseDepth,
            .mipLevels = kTexture->numLevels,
            .layers = kTexture->numFaces,
            .elements = kTexture->numLayers,
            .usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT,
        };

        Gfx::OTexture2D texture = buffer.getGraphics()->createTexture2D(createInfo);
        usedTextures.add(std::move(texture));

        ktxTexture_Destroy(ktxTexture(kTexture));
    }
}

void Seele::FontAsset::setUsedTextures(Array<Gfx::OTexture2D> _usedTextures) 
{ 
    usedTextures = std::move(_usedTextures); 
}

void FontAsset::Glyph::save(ArchiveBuffer& buffer) const
{
    Serialization::save(buffer, textureIndex);
    Serialization::save(buffer, size);
    Serialization::save(buffer, bearing);
    Serialization::save(buffer, advance);
}

void FontAsset::Glyph::load(ArchiveBuffer& buffer)
{
    Serialization::load(buffer, textureIndex);
    Serialization::load(buffer, size);
    Serialization::load(buffer, bearing);
    Serialization::load(buffer, advance);
}
