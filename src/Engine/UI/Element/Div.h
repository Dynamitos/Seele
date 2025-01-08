#pragma once
#include "UI/Element.h"
#include "UI/Style/Class.h"

namespace Seele {
namespace UI {
template<StyleClass... classes>
class Div : public Element {
  public:
    Div(Attributes attr, std::initializer_list<OElement> children) : Element(attr, children) { style.displayType = DisplayType::Block; }
    virtual ~Div() {}
  private:
};
} // namespace UI
} // namespace Seele