#include "TextureAsset.h"
#include "Graphics/GraphicsResources.h"
#include "Graphics/Graphics.h"
#include "Window/WindowManager.h"
#include <stb_image.h>
#include <stb_image_write.h>

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
    int x, y, n;
    unsigned char* data = stbi_load(getFullPath().c_str(), &x, &y, &n, 4);
    TextureCreateInfo createInfo;
    
    //TODO: support other formats
    createInfo.format = Gfx::SE_FORMAT_R8G8B8A8_UINT;
    createInfo.resourceData.data = data;
    createInfo.resourceData.size = x * y * 4 * sizeof(unsigned char);
    createInfo.resourceData.owner = Gfx::QueueType::DEDICATED_TRANSFER;
    createInfo.width = x;
    createInfo.height = y;
    Gfx::PTexture2D texture = WindowManager::getGraphics()->createTexture2D(createInfo);
    stbi_image_free(data);
    texture->transferOwnership(Gfx::QueueType::GRAPHICS);
    setTexture(texture);
    setStatus(Asset::Status::Ready);
}