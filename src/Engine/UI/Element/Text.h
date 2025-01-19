#pragma once
#include "UI/Element.h"
#include "UI/Style/Class.h"

namespace Seele {
namespace UI {
template <StyleClass... classes> class Text : public Element {
  public:
    Text(std::string text) : Element({}, {}), text(text) {}
    virtual void applyStyle(Style parentStyle) {
        style = parentStyle;
        style.outerDisplay = OuterDisplayType::Inline;
        (classes::apply(style), ...);
    }
    virtual void layout(UVector2 parentSize) override {
        size_t cursor = 0;
        for (uint32 i = 0; i < text.size() - 1; ++i) {
            dimensions.x += style.fontFamily->getGlyphData(text[i], style.fontSize).advance / 64.0f;
        }
        dimensions.x += style.fontFamily->getGlyphData(text[text.size() - 1], style.fontSize).size.x;
        dimensions.y = style.fontSize;
    }
    virtual Array<UIRender> render(Vector2 anchor, uint32 level) override {
        return {
            UIRender{
                .text = text,
                .font = style.fontFamily,
                .fontSize = style.fontSize,
                .position = position + anchor,
                .dimensions = dimensions,
                .baseline = style.fontSize * 3 / 4, // TODO: improve
                .z = style.z,
                .level = level,
            },
        };
    }
    // height = fontsize * 1.12

  private:
    std::string text;
};
} // namespace UI
} // namespace Seele