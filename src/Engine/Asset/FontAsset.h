#pragma once
#include "Asset.h"
#include "Containers/Map.h"
#include "Math/Math.h"
#include <freetype/freetype.h>

namespace Seele {
DECLARE_NAME_REF(Gfx, Texture2D)
class FontAsset : public Asset {
  public:
    static constexpr uint64 IDENTIFIER = 0x10;
    FontAsset();
    FontAsset(std::string_view folderPath, std::string_view name);
    virtual ~FontAsset();
    virtual void save(ArchiveBuffer& buffer) const override;
    virtual void load(ArchiveBuffer& buffer) override;

    struct Glyph {
        UVector2 size = {};
        UVector2 bearing = {};
        uint32 advance = 0;
        Array<uint8> textureData;
        Gfx::OTexture2D texture = nullptr;
        void save(ArchiveBuffer& buffer) const;
        void load(ArchiveBuffer& buffer);
    };
    const Glyph& getGlyphData(char c) const { return glyphs[c]; }

  private:
    Map<uint32, Glyph> glyphs;
    friend class FontLoader;
};
DECLARE_REF(FontAsset)
} // namespace Seele
