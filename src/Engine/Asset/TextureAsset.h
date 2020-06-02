#include "Asset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture);
class TextureAsset : public Asset
{
public:
    TextureAsset();
    TextureAsset(const std::string& directory, const std::string& name);
    TextureAsset(const std::string& fullPath);
    void setTexture(Gfx::PTexture texture)
    {
        std::scoped_lock lck(lock);
        this->texture = texture;
    }
    Gfx::PTexture getTexture()
    {
        return texture;
    }
private:
    Gfx::PTexture texture;
};
DEFINE_REF(TextureAsset);
} // namespace Seele
