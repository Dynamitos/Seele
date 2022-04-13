#pragma once
#include "Asset.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
class FontAsset : public Asset
{
public:
    FontAsset();
    FontAsset(const std::string& directory, const std::string& name);
    FontAsset(const std::filesystem::path& fullPath);
    virtual ~FontAsset();
    virtual void save() override;
    virtual void load() override;
private:
    struct Glyph
    {
        int16 index;
        int16 numContours;
        uint16 advance;
        int16 leftSideBearing;
        int16 boundingBox[4];
        Vector2 center;
        Gfx::PTexture2D curveTexture;
        Gfx::PTexture2D bandTexture;
    };
    Map<uint32, Glyph> glyphs;
};
DECLARE_REF(FontAsset)
} // namespace Seele
