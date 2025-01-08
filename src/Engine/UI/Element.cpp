#include "Element.h"
#include "System.h"

using namespace Seele;
using namespace Seele::UI;

Array<RenderElement> Element::render() {
    RenderElement result;
    result.position = UVector2(0, 0);
    result.size = UVector2(style.width, style.height);
    result.backgroundColor = style.backgroundColor;
    result.fontSize = style.fontSize;
    result.fontFamily = style.fontFamily;
    return {result};
}

Element::Element(Attributes attr, Array<OElement> children) : attr(attr), children(std::move(children)) {}

Element::~Element() {}