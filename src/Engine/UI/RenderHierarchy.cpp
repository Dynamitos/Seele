#include "RenderHierarchy.h"

using namespace Seele;
using namespace Seele::UI;

void AddElementRenderHierarchyUpdate::apply(Array<RenderElement>& elements) 
{
    auto parentIt = elements.find([this](RenderElement e){return e.parent == parent;});
    parentIt->referencedElement->addChild(addedElement);
    addedElement->setParent(parentIt->referencedElement);

    elements.emplace(parentIt->referencedElement, addedElement);
}

void RemoveElementRenderHierarchyUpdate::apply(Array<RenderElement>& elements) 
{
    auto elementIt = elements.find([this](RenderElement e){return e.referencedElement == element;});
    if(!removeChildren)
    {
        for(auto child : elementIt->referencedElement->getChildren())
        {
            child->setParent(elementIt->referencedElement->getParent());
        }
    }
    elementIt->referencedElement->setParent(nullptr);

    elements.remove(elementIt);
}

RenderHierarchy::RenderHierarchy() 
{
    
}

RenderHierarchy::~RenderHierarchy() 
{
    
}

void RenderHierarchy::addElement(PElement addedElement) 
{
    std::scoped_lock lock(updateLock);
    updates.add(new AddElementRenderHierarchyUpdate{
        addedElement.getHandle(), 
        addedElement->getParent().getHandle()
    });
}

void RenderHierarchy::removeElement(PElement elementToRemove) 
{
    std::scoped_lock lock(updateLock);
    updates.add(new RemoveElementRenderHierarchyUpdate{
        elementToRemove.getHandle(),
    });
}

void RenderHierarchy::moveElement(PElement elementToMove, PElement newParent) 
{
    std::scoped_lock lock(updateLock);
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
    List<RenderHierarchyUpdate*> localUpdates;
    { // make a local copy of the updates so we dont hold the lock for too long
        std::scoped_lock lock(updateLock);
        localUpdates = updates;
        updates.clear();
    }
    for(auto update : localUpdates)
    {
        update->apply(drawElements);
    }
}
