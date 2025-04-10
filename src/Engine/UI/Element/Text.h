#pragma once
#include "UI/Element.h"
#include "UI/Style/Class.h"
#include <ranges>

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
    virtual void layout(UVector2) override {
        UVector2 cursor = UVector2(0);
        dimensions = style.fontFamily->shapeText(text, style.fontSize, glyphRenders);
    }
    virtual Array<UIRender> render(Vector2 anchor, uint32 level) override {
        return Array<UIRender>(std::from_range, glyphRenders | std::views::transform([=](const FontAsset::RenderGlyph& glyph) {
                                          return UIRender{
                                              .backgroundTexture = glyph.texture,
                                              .position = Vector2(glyph.position) + anchor,
                                              .dimensions = Vector2(glyph.dimensions),
                                              .level = level,
                                          };
                                      }));
    }
    // height = fontsize * 1.12

  private:
    std::string text;
    Array<FontAsset::RenderGlyph> glyphRenders;
};
} // namespace UI
} // namespace Seele