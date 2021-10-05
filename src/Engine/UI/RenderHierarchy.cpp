#include "RenderHierarchy.h"

using namespace Seele;
using namespace Seele::UI;

void AddElementRenderHierarchyUpdate::apply(Array<RenderElement>& elements) 
{
    for(auto element : elements)
    {
        if(element.parent == parent)
        {
            
        }
    }
}

void RemoveElementRenderHierarchyUpdate::apply(Array<RenderElement>& elements) 
{
    
}

RenderHierarchy::RenderHierarchy() 
{
    
}

RenderHierarchy::~RenderHierarchy() 
{
    
}

void RenderHierarchy::addElement(PElement addedElement) 
{
    std::lock_guard lock(updateLock);
    updates.add(new AddElementRenderHierarchyUpdate{
        addedElement.getHandle(), 
        addedElement->getParent().getHandle()
    }));
}

void RenderHierarchy::removeElement(PElement elementToRemove) 
{
    std::lock_guard lock(updateLock);
    updates.add(new RemoveElementRenderHierarchyUpdate{
        elementToRemove.getHandle(),
    });
}

void RenderHierarchy::moveElement(PElement elementToMove, PElement newParent) 
{
    std::lock_guard lock(updateLock);
    updates.add(new AddElementRenderHierarchyUpdate{
        elementToMove.getHandle(),
        newParent.getHandle()
    });
    updates.add(new RemoveElementRenderHierarchyUpdate{
        elementToMove.getHandle()
    });
}

void RenderHierarchy::updateHierarchy() 
{
    Array<RenderHierarchyUpdate*> localUpdates;
    { // make a local copy of the updates so we dont hold the lock for too long
        std::lock_guard lock(updateLock);
        localUpdates = updates;
        updates.clear();
    }
    for(auto update : localUpdates)
    {
        
    }
}
