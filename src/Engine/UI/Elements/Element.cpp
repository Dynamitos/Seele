#include "Element.h"
#include "UI/System.h"

using namespace Seele;
using namespace Seele::UI;

Element::Element()
    : dirty(false)
    , enabled(false)
{
}

Element::~Element() 
{
}

PElement Element::getParent() const
{
    return parent;
}

void Element::addChild(PElement element) 
{
    children.add(element);
}

const Array<PElement> Element::getChildren()
{
    return children;
}

void Element::clear() 
{
    children.clear();
}

void Element::remove(PElement element) 
{
    children.remove(children.find(element));
}

void Element::setEnabled(bool newEnabled) 
{
    enabled = newEnabled;
}

bool Element::isEnabled() const
{
    return enabled;
}

Rect& Element::getBoundingBox() 
{
    return boundingBox;
}

const Rect Element::getBoundingBox() const
{
    return boundingBox;
}