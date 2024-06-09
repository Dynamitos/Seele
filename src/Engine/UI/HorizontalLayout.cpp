#include "HorizontalLayout.h"
#include "Elements/Element.h"

using namespace Seele;
using namespace Seele::UI;

HorizontalLayout::HorizontalLayout(PElement element) : Layout(element) {}

HorizontalLayout::~HorizontalLayout() {}

void HorizontalLayout::apply() {
    Array<PElement> children = element->getChildren();
    const Rect parent = element->getBoundingBox();
    float xOffset = parent.offset.x;
    float yOffset = parent.offset.y;
    float xSize = parent.size.x / children.size();
    float ySize = parent.size.y;
    for (uint32 index = 0; index < children.size(); ++index) {
        Rect& child = children[index]->getBoundingBox();
        child.offset.x = xOffset + (index * xSize);
        child.offset.y = yOffset;
        child.size.x = xSize;
        child.size.y = ySize;
    }
}