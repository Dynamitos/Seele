#pragma once
#include "MinimalEngine.h"
#include "Style.h"

namespace Seele {
namespace UI {
template <typename C>
concept StyleClass = requires(C c, Style& s) { c.apply(s); };

template <uint32 w, DimensionType widthType> struct WidthClass {
    static void apply(Style& s) {
        s.width = w;
        s.widthType = widthType;
    }
};
using W_Full = WidthClass<100, DimensionType::Percent>;
using W_1 = WidthClass<2, DimensionType::Pixel>;

template <DisplayType displayType> struct DisplayClass {
    static void apply(Style& s) { s.displayType = displayType; }
};
using Hidden = DisplayClass<DisplayType::Hidden>;
using Block = DisplayClass<DisplayType::Block>;
using Flex = DisplayClass<DisplayType::Flex>;

} // namespace UI
} // namespace Seele