#include "FontAsset.h"
#include "Graphics/Graphics.h"
#include "Graphics/Texture.h"

using namespace Seele;

FontAsset::FontAsset() {}

FontAsset::FontAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {}

FontAsset::~FontAsset() {}

void FontAsset::save(ArchiveBuffer& buffer) const { Serialization::save(buffer, glyphs); }

void FontAsset::load(ArchiveBuffer& buffer) { Serialization::load(buffer, glyphs); }

void FontAsset::Glyph::save(ArchiveBuffer& buffer) const {
    Serialization::save(buffer, size);
    Serialization::save(buffer, bearing);
    Serialization::save(buffer, advance);
    Serialization::save(buffer, textureData);
}

void FontAsset::Glyph::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, size);
    Serialization::load(buffer, bearing);
    Serialization::load(buffer, advance);
    Serialization::load(buffer, textureData);
    texture = buffer.getGraphics()->createTexture2D(TextureCreateInfo{
        .sourceData =
            {
                .size = textureData.size(),
                .data = textureData.data(),
            },
        .format = Gfx::SE_FORMAT_R8_UNORM,
        .width = size.x,
        .height = size.y,
        .name = "FontGlyph",
    });
}
