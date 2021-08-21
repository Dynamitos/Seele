#include "Element.h"

using namespace Seele;
using namespace Seele::UI;

Element::Element()
{
}

Element::~Element() 
{
}

void Element::addElement(PElement element) 
{
    children.add(element);   
}

const Array<PElement> Element::getChildren() const
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