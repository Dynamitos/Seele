#include "RenderHierarchy.h"

using namespace Seele;
using namespace Seele::UI;

RenderElement::RenderElement() 
{
    
}

RenderElement::~RenderElement() 
{
    
}

RenderHierarchy::RenderHierarchy() 
{
    
}

RenderHierarchy::~RenderHierarchy() 
{
    
}

void RenderHierarchy::updateHierarchyIndices() 
{
    for (uint32 i = 0; i < drawElements.size(); i++)
    {
        drawElements[i].hierarchyIndex = i;
    }
}
