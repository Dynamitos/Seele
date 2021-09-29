#pragma once
#include "Elements/Element.h"

namespace Seele
{
namespace UI
{
DECLARE_NAME_REF(Gfx, RenderCommand);
class RenderElement
{
public:
    RenderElement();
    virtual ~RenderElement();
private:
    uint32 hierarchyIndex;
    RenderElement* parent;
    PElement referencedElement;
    Gfx::PRenderCommand renderCommand;
    friend class RenderHierarchy;
};

struct RenderHierarchyUpdate
{};
DEFINE_REF(RenderHierarchyUpdate)


class RenderHierarchy
{
public:
    RenderHierarchy();
    ~RenderHierarchy();
    
private:
    void updateHierarchyIndices();

    // List of all drawable elements in draw order
    Array<RenderElement> drawElements;
};
} // namespace UI
} // namespace Seele
