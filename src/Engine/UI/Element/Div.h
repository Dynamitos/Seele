#pragma once
#include "UI/Element.h"
#include "UI/Style/Class.h"

namespace Seele {
namespace UI {
template<StyleClass... classes>
class Div : public Element {
  public:
    Div(Attributes attr, Array<Element*> children) : Element(attr, std::move(children)) { }
    virtual ~Div() {}
    virtual void applyStyle(Style parentStyle) override {
        style = parentStyle;
        style.outerDisplay = OuterDisplayType::Block;
        style.widthType = DimensionType::Percent;
        style.width = 100;
        (classes::apply(style), ...);
    }
  private:
};
} // namespace UI
} // namespace Seele