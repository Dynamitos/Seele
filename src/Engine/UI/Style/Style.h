#pragma once
#include "Math/Vector.h"
#include "Asset/FontAsset.h"
#include "MinimalEngine.h"

namespace Seele {
namespace UI {
enum class DimensionType {
    Pixel,
    Percent, // EM, PEM, etc...
    Auto,
};
enum class PositionType {
    Static,
    Relative,
    Absolute,
    Sticky,
};
enum class OuterDisplayType {
    Inline,
    Block,
    Hidden,
};
enum class InnerDisplayType {
    Flow,
    Flex,
    Grid,
};
struct Style {
    DimensionType widthType = DimensionType::Auto;
    float width = 0;
    DimensionType maxWidthType = DimensionType::Auto;
    float maxWidth = 0;
    DimensionType heightType = DimensionType::Auto;
    float height = 0;
    DimensionType maxHeightType = DimensionType::Auto;
    float maxHeight = 0;
    uint32 z = 0;
    Vector backgroundColor = Vector(1, 1, 1);
    OuterDisplayType outerDisplay = OuterDisplayType::Inline;
    InnerDisplayType innerDisplay = InnerDisplayType::Flow;
    PositionType position = PositionType::Static;
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
    uint32 top = 0;
    uint32 bottom = 0;
    uint32 left = 0;
    uint32 right = 0;
};
} // namespace UI
} // namespace Seele