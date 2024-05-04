#include "TextureAsset.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "Graphics/Vulkan/Enums.h"
#include "Graphics/Texture.h"
#include "ktx.h"

using namespace Seele;

#define KTX_ASSERT(x) { auto error = x; if(error != KTX_SUCCESS) { std::cout << ktxErrorString(error) << std::endl; abort(); } }

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
    //ktxBasisParams basisParams = {
    //    .structSize = sizeof(ktxBasisParams),
    //    .uastc = true,
    //    .threadCount = std::thread::hardware_concurrency(),
    //    .normalMap = normalMap,
    //    .uastcFlags = KTX_PACK_UASTC_LEVEL_VERYSLOW,
    //    .uastcRDO = true,
    //    .uastcRDOQualityScalar = 1,
    //};
    //KTX_ASSERT(ktxTexture2_CompressBasisEx(ktxHandle, &basisParams));
    //KTX_ASSERT(ktxTexture2_DeflateZstd(ktxHandle, 20));
    //ktx_uint8_t* texData;
    //ktx_size_t texSize;
    //KTX_ASSERT(ktxTexture_WriteToMemory(ktxTexture(ktxHandle), &texData, &texSize));
    //
    //Array<uint8> rawData(texSize);
    //std::memcpy(rawData.data(), texData, texSize);
    //Serialization::save(buffer, rawData);
    //free(texData);
}

void TextureAsset::load(ArchiveBuffer& buffer) 
{
    std::string ktxPath;
    Serialization::load(buffer, ktxPath);
    ktxTexture2* ktxHandle;
    
    KTX_ASSERT(ktxTexture_CreateFromNamedFile(ktxPath.c_str(),
        KTX_TEXTURE_CREATE_NO_FLAGS,
        (ktxTexture**)&ktxHandle));

    KTX_ASSERT(ktxTexture2_TranscodeBasis(ktxHandle, KTX_TTF_BC7_RGBA, 0));

    Gfx::PGraphics graphics = buffer.getGraphics();
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
    ktxTexture_Destroy(ktxTexture(ktxHandle));
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
