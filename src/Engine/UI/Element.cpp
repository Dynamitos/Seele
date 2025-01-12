#include "Element.h"
#include "System.h"

using namespace Seele;
using namespace Seele::UI;

Element::Element(Attributes attr, Array<Element*> _children) : attr(attr) {
    for (auto c : _children) {
        children.add(c);
    }
    for (auto& child : children) {
        child->setParent(this);
    }
}

Element::~Element() {}
