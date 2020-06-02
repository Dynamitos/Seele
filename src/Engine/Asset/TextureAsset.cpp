#include "TextureAsset.h"
#include "Graphics/GraphicsResources.h"

using namespace Seele;


TextureAsset::TextureAsset()
{
}
TextureAsset::TextureAsset(const std::string& directory, const std::string& name)
    : Asset(directory, name)
{
}
TextureAsset::TextureAsset(const std::string& fullPath)
    : Asset(fullPath)
{
}