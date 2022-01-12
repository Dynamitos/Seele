#include "Asset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture)
class TextureAsset : public Asset
{
public:
    TextureAsset();
    TextureAsset(const std::string& directory, const std::string& name);
    TextureAsset(const std::filesystem::path& fullPath);
    virtual ~TextureAsset();
    virtual void save() override;
    virtual void load() override;
    void setTexture(Gfx::PTexture texture)
    {
        std::scoped_lock lck(lock);
        this->texture = texture;
    }
    Gfx::PTexture getTexture()
    {
        std::scoped_lock lck(lock);
        return texture;
    }
private:
    Gfx::PTexture texture;
    friend class TextureLoader;
};
DEFINE_REF(TextureAsset)
} // namespace Seele
