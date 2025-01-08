#pragma once
#include "Math/Vector.h"
#include "Asset/FontAsset.h"
#include "MinimalEngine.h"

namespace Seele {
namespace UI {
enum class DimensionType {
    Pixel,
    Percent, // EM, PEM, etc...
};
enum class PositionType {
    Relative,
    Static,
    Absolute,
    Sticky,
};
enum class DisplayType {
    Inline,
    Hidden,
    Block,
    Flex,
    Grid,
};
struct Style {
    DimensionType widthType = DimensionType::Pixel;
    uint32 width = 0;
    DimensionType heightType = DimensionType::Pixel;
    uint32 height = 0;
    uint32 z = 0;
    Vector backgroundColor = Vector(1, 1, 1);
    DisplayType displayType = DisplayType::Inline;
    PositionType position = PositionType::Relative;
    PFontAsset fontFamily;
    uint32 lineHeight = 12;
    uint32 fontSize = 12;
    uint32 fontWeight = 0;
    uint32 paddingTop = 0;
    uint32 paddingBottom = 0;
    uint32 paddingLeft = 0;
    uint32 paddingRight = 0;
    uint32 marginTop = 0;
    uint32 marginBottom = 0;
    uint32 marginLeft = 0;
    uint32 marginRight = 0;
    uint32 gap = 0;
};
} // namespace UI
} // namespace Seele