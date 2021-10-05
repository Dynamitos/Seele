#pragma once
#include "MinimalEngine.h"

namespace Seele
{
namespace UI
{
//Element defines any part of the UI
DECLARE_REF(Element)
DECLARE_REF(System)
class Element
{
public:
    Element();
    virtual ~Element();
    void setParent(PElement element);
    PElement getParent() const;
    void addChild(PElement element);
    const Array<PElement> getChildren();
    void clear();
    void remove(PElement element);
    void setEnabled(bool newEnabled);
    bool isEnabled() const;
    // maybe not the healthiest inteface
    // non-const version
    Rect& getBoundingBox();
    // The bounding box describes the relative size of any Element
    // relative to the total view, meaning a bounding box of (0,0), (1,1)
    // would take up the entire view
    const Rect getBoundingBox() const;
protected:
    Rect boundingBox;
    bool dirty;
    
    bool enabled;
    PSystem system;
    PElement parent;
    Array<PElement> children;
    friend class Layout;
    friend class RenderElement;
};
DEFINE_REF(Element)
} // namespace UI
} // namespace Seele
