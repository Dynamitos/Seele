#include "Element.h"
#include "Graphics/Graphics.h"
#include "UI/System.h"

using namespace Seele;
using namespace Seele::UI;

Element::Element() : dirty(false), enabled(false) {}

Element::~Element() {}

void Element::setParent(PElement element) {
    if (parent != nullptr) {
        parent->removeChild(this);
    }
    parent = element;
}

PElement Element::getParent() const { return parent; }

void Element::addChild(PElement element) { children.add(element); }

void Element::removeChild(PElement element) { children.remove(element, false); }

const Array<PElement> Element::getChildren() { return children; }

void Element::clear() { children.clear(); }

void Element::setEnabled(bool newEnabled) { enabled = newEnabled; }

bool Element::isEnabled() const { return enabled; }

Rect& Element::getBoundingBox() { return boundingBox; }

const Rect Element::getBoundingBox() const { return boundingBox; }