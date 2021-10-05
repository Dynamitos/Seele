#pragma once
#include "Elements/Element.h"
#include "Containers/List.h"

namespace Seele
{
namespace UI
{
DECLARE_NAME_REF(Gfx, RenderCommand);
class RenderElement
{
public:
    RenderElement() = default;
    ~RenderElement() = default;
    uint32 hierarchyIndex;
    Element* parent;
    Element* referencedElement;
};

struct RenderHierarchyUpdate
{
    virtual void apply(Array<RenderElement>& elements) = 0;
};

struct AddElementRenderHierarchyUpdate : public RenderHierarchyUpdate
{
    Element* addedElement;
    Element* parent;
    virtual void apply(Array<RenderElement>& elements) override;
};

struct RemoveElementRenderHierarchyUpdate : public RenderHierarchyUpdate
{
    Element* element;
    virtual void apply(Array<RenderElement>& elements) override;
};

class RenderHierarchy
{
public:
    RenderHierarchy();
    ~RenderHierarchy();
    // logic thread interface, queue hierarchy changes
    void addElement(PElement addedElement);
    void removeElement(PElement elementToRemove);
    void moveElement(PElement elementToMove, PElement newParent);

    // render thread interface, apply changes
    void updateHierarchy();
private:
    static_assert(std::is_trivially_copyable_v<RenderElement>);
    // List of all drawable elements in draw order
    Array<RenderElement> drawElements;

    List<RenderHierarchyUpdate*> updates;
    std::mutex updateLock;
};
} // namespace UI
} // namespace Seele
