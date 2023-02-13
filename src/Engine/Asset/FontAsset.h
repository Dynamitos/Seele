#pragma once
#include "Asset.h"
#include "Containers/Map.h"
#include "Math/Math.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Texture2D)
class FontAsset : public Asset
{
public:
    static constexpr uint64 IDENTIFIER = 0x10;
    FontAsset();
    FontAsset(std::string_view folderPath, std::string_view name);
    virtual ~FontAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;

    struct Glyph
    {
        Gfx::PTexture2D texture;
        IVector2 size;
        IVector2 bearing;
        uint32 advance;
    };
    const Map<uint32, Glyph> getGlyphData() const { return glyphs; }
private:
    Map<uint32, Glyph> glyphs;
    friend class FontLoader;
};
DECLARE_REF(FontAsset)
} // namespace Seele
