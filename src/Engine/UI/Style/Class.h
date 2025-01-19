#pragma once
#include "Asset/AssetRegistry.h"
#include "MinimalEngine.h"
#include "Style.h"

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
using Font_Calibri = FontClass<"Calibri">;

template <uint32 fontSize, uint32 lineHeight> struct FontSizeClass {
    static void apply(UI::Style& s) {
        s.fontSize = fontSize;
        s.lineHeight = lineHeight;
    }
};

using Text_XS = FontSizeClass<12, 16>;
using Text_SM = FontSizeClass<14, 20>;
using Text_Base = FontSizeClass<16, 24>;
using Text_LG = FontSizeClass<18, 28>;
using Text_XL = FontSizeClass<20, 28>;
using Text_2XL = FontSizeClass<24, 32>;
using Text_3XL = FontSizeClass<30, 36>;
using Text_4XL = FontSizeClass<36, 40>;
using Text_5XL = FontSizeClass<48, 48>;
using Text_6XL = FontSizeClass<60, 60>;
using Text_7XL = FontSizeClass<72, 72>;
using Text_8XL = FontSizeClass<96, 96>;
using Text_9XL = FontSizeClass<128, 128>;

template <Vector backgroundColor> struct BackgroundColorClass {
    static void apply(UI::Style& s) { s.backgroundColor = backgroundColor; }
};

using BG_Red = BackgroundColorClass<Vector(1, 0, 0)>;

template <uint32 ml, uint32 mr, uint32 mt, uint32 mb> struct MarginClass {
    static void apply(UI::Style& s) {
        if (ml != -1)
            s.marginLeft = ml;
        if (mr != -1)
            s.marginRight = mr;
        if (mt != -1)
            s.marginTop = mt;
        if (mb != -1)
            s.marginBottom = mb;
    }
};
#define DECLARE_MARGIN_CLASSES(x)                                                                                                          \
    using M_##x = MarginClass<x, x, x, x>;                                                                                                 \
    using M_X##x = MarginClass<x, x, -1, -1>;                                                                                              \
    using M_Y##x = MarginClass<-1, -1, x, x>;                                                                                              \
    using M_L##x = MarginClass<x, -1, -1, -1>;                                                                                             \
    using M_R##x = MarginClass<-1, x, -1, -1>;                                                                                             \
    using M_T##x = MarginClass<-1, -1, x, -1>;                                                                                             \
    using M_B##x = MarginClass<-1, -1, -1, x>;

DECLARE_MARGIN_CLASSES(0)
DECLARE_MARGIN_CLASSES(1)
DECLARE_MARGIN_CLASSES(2)
DECLARE_MARGIN_CLASSES(3)
DECLARE_MARGIN_CLASSES(4)
DECLARE_MARGIN_CLASSES(5)
DECLARE_MARGIN_CLASSES(6)
DECLARE_MARGIN_CLASSES(7)
DECLARE_MARGIN_CLASSES(8)
DECLARE_MARGIN_CLASSES(9)
DECLARE_MARGIN_CLASSES(10)
DECLARE_MARGIN_CLASSES(11)
DECLARE_MARGIN_CLASSES(12)
DECLARE_MARGIN_CLASSES(14)
DECLARE_MARGIN_CLASSES(16)
DECLARE_MARGIN_CLASSES(20)
DECLARE_MARGIN_CLASSES(24)


} // namespace Seele