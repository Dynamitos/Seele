#include "VerticalLayout.h"
#include "Elements/Element.h"

using namespace Seele;
using namespace Seele::UI;

VerticalLayout::VerticalLayout(PElement element) : Layout(element) {}

VerticalLayout::~VerticalLayout() {}

void VerticalLayout::apply() {
    Array<PElement> children = element->getChildren();
    const Rect parent = element->getBoundingBox();
    float xOffset = parent.offset.x;
    float yOffset = parent.offset.y;
    float xSize = parent.size.x;
    float ySize = parent.size.y / children.size();
    for (uint32 index = 0; index < children.size(); ++index) {
        Rect& child = children[index]->getBoundingBox();
        child.offset.x = xOffset;
        child.offset.y = yOffset + (index * ySize);
        child.size.x = xSize;
        child.size.y = ySize;
    }
}