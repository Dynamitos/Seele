#pragma once
#include "MinimalEngine.h"
#include "Style.h"
#include "Asset/AssetRegistry.h"

namespace Seele {
template <typename C>
concept StyleClass = requires(C c, UI::Style& s) { c.apply(s); };

template <uint32 w, UI::DimensionType widthType> struct WidthClass {
    static void apply(UI::Style& s) {
        s.width = w;
        s.widthType = widthType;
    }
};
using W_Full = WidthClass<100, UI::DimensionType::Percent>;
using W_1 = WidthClass<2, UI::DimensionType::Pixel>;

template <UI::OuterDisplayType outer, UI::InnerDisplayType inner> struct DisplayClass {
    static void apply(UI::Style& s) {
        s.outerDisplay = outer;
        s.innerDisplay = inner;
    }
};
using Hidden = DisplayClass<UI::OuterDisplayType::Hidden, UI::InnerDisplayType::Flow>;
using Block = DisplayClass<UI::OuterDisplayType::Block, UI::InnerDisplayType::Flow>;
using Inline = DisplayClass<UI::OuterDisplayType::Inline, UI::InnerDisplayType::Flow>;

template <size_t N> struct StringLiteral {
    constexpr StringLiteral(const char (&str)[N]) { std::copy_n(str, N, value); }

    char value[N];
};
template <StringLiteral lit> struct FontClass {
    static void apply(UI::Style& s) { s.fontFamily = AssetRegistry::findFont("", lit.value); }
};

using Font_Arial = FontClass<"arial">;

template <Vector backgroundColor> struct BackgroundColorClass {
    static void apply(UI::Style& s) { s.backgroundColor = backgroundColor; }
};

using BG_Red = BackgroundColorClass<Vector(1, 0, 0)>;
} // namespace Seele