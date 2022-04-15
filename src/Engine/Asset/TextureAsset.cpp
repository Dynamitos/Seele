#include "TextureAsset.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include "Graphics/Vulkan/VulkanGraphicsEnums.h"
#include "ktx.h"

using namespace Seele;


TextureAsset::TextureAsset()
{
}
TextureAsset::TextureAsset(const std::string& directory, const std::string& name)
    : Asset(directory, name)
{
}
TextureAsset::TextureAsset(const std::filesystem::path& fullPath) 
    : Asset(fullPath)
{   
}

TextureAsset::~TextureAsset() 
{
    
}

void TextureAsset::save() 
{
    //TODO: make this an actual file, not just a png wrapper
    assert(false && "Editing textures is not yet supported");
}

void TextureAsset::load() 
{
    setStatus(Status::Loading);
    ktxTexture2* kTexture;
    // TODO: consider return
    std::cout << "Loading texture " << getFullPath() << std::endl;
    ktxTexture2_CreateFromNamedFile(getFullPath().c_str(), 
        KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, 
        &kTexture);
    TextureCreateInfo createInfo;
    createInfo.width = kTexture->baseWidth;
    createInfo.height = kTexture->baseHeight;
    createInfo.depth = kTexture->baseDepth;
    createInfo.bArray = kTexture->isArray;
    createInfo.arrayLayers = kTexture->numLayers;
    createInfo.format = Vulkan::cast((VkFormat)kTexture->vkFormat);
    createInfo.mipLevels = kTexture->numLevels;
    createInfo.usage = Gfx::SE_IMAGE_USAGE_SAMPLED_BIT;
    createInfo.resourceData.data = ktxTexture_GetData(ktxTexture(kTexture));
    createInfo.resourceData.size = (uint32)ktxTexture_GetDataSize(ktxTexture(kTexture));
    createInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    Gfx::PTexture2D texture = WindowManager::getGraphics()->createTexture2D(createInfo);
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
    setTexture(texture);
    setStatus(Asset::Status::Ready);
}