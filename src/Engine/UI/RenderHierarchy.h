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
    PElement referencedElement;
    Gfx::PRenderCommand renderCommand;
    friend class RenderHierarchy;
};
class RenderHierarchy
{
public:
    RenderHierarchy();
    ~RenderHierarchy();
private:
    // List of all drawable elements in draw order
    Array<RenderElement> drawElements;
};
} // namespace UI
} // namespace Seele
