#include "SVGAsset.h"
#include "Graphics/Graphics.h"

using namespace Seele;

SVGAsset::SVGAsset() {}

SVGAsset::SVGAsset(std::string_view folderPath, std::string_view name) : Asset(folderPath, name) {}

SVGAsset::~SVGAsset() {}

void SVGAsset::save(ArchiveBuffer& buffer) const { Serialization::save(buffer, data); }

void SVGAsset::load(ArchiveBuffer& buffer) {
    Serialization::load(buffer, data);
    document = lunasvg::Document::loadFromData(data.data());
}

Gfx::PTexture2D SVGAsset::getTexture(UVector2 dimensions) {
    auto viewbox = ViewBox{
        .width = dimensions.x,
        .height = dimensions.y,
    };
    if (!cachedTextures.contains(viewbox)) {
        auto bitmap = document->renderToBitmap(viewbox.width, viewbox.height);
        bitmap.convertToRGBA();
        cachedTextures[viewbox] = graphics->createTexture2D(TextureCreateInfo{
            .format = Gfx::SE_FORMAT_R32G32B32A32_SFLOAT,
            .width = viewbox.width,
            .height = viewbox.height,
            .name = "SVGTexture",
        });
    }
    return cachedTextures[viewbox];
}
