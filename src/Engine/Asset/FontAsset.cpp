#include "FontAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Vulkan/VulkanGraphicsEnums.h"
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
    uint64 numGlyphs = glyphs.size();
    Serialization::save(buffer, numGlyphs);
    for (auto& [index, glyph] : glyphs)
    {

        Array<uint8> textureData;
        ktxTexture2* kTexture;
        ktxTextureCreateInfo createInfo = {
            .vkFormat = (uint32_t)glyph.texture->getFormat(),
            .baseWidth = glyph.texture->getSizeX(),
            .baseHeight = glyph.texture->getSizeY(),
            .baseDepth = glyph.texture->getSizeZ(),
            .numDimensions = glyph.texture->getSizeZ() > 1 ? 3u : glyph.texture->getSizeY() > 1 ? 2u : 1u,
            .numLevels = glyph.texture->getMipLevels(),
            .numLayers = glyph.texture->getSizeZ(),
            .numFaces = glyph.texture->getNumFaces(),
            .isArray = false,
            .generateMipmaps = false,
        };
        ktxTexture2_Create(&createInfo, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &kTexture);

        for (uint32 depth = 0; depth < glyph.texture->getSizeZ(); ++depth)
        {
            for (uint32 face = 0; face < glyph.texture->getNumFaces(); ++face)
            {
                // technically, downloading cant be const, because we have to allocate temporary buffers and change layouts
                // but practically the texture stays the same
                glyph.texture->download(0, depth, face, textureData);
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

        Serialization::save(buffer, index);
        Serialization::save(buffer, rawData);
        Serialization::save(buffer, glyph.size);
        Serialization::save(buffer, glyph.bearing);
        Serialization::save(buffer, glyph.advance);
    }
}


void FontAsset::load(ArchiveBuffer& buffer)
{
    uint64 numGlyphs = glyphs.size();
    Serialization::load(buffer, numGlyphs);
    for (uint64 x = 0; x < numGlyphs; ++x)
    {
        uint32 index;
        Serialization::load(buffer, index);

        Glyph& glyph = glyphs[index];

        Array<uint8> rawTex;
        Serialization::load(buffer, rawTex);

        ktxTexture2* kTexture;
        ktxTexture2_CreateFromMemory(rawTex.data(),
            rawTex.size(),
            KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT,
            &kTexture);

        ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_BC7_RGBA, 0);

        TextureCreateInfo createInfo = {
            .resourceData = {
                .size = ktxTexture_GetDataSize(ktxTexture(kTexture)),
                .data = ktxTexture_GetData(ktxTexture(kTexture)),
                .owner = Gfx::QueueType::GRAPHICS,
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

        glyph.texture = buffer.getGraphics()->createTexture2D(createInfo);

        ktxTexture_Destroy(ktxTexture(kTexture));

        Serialization::load(buffer, glyph.size);
        Serialization::load(buffer, glyph.bearing);
        Serialization::load(buffer, glyph.advance);
    }
}