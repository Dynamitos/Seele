#pragma once
#include "Attributes.h"
#include "Containers/Array.h"
#include "MinimalEngine.h"
#include "Style/Style.h"

namespace Seele {
namespace UI {
struct RenderElement;
DECLARE_REF(Element)
class Element {
  public:
    Element(Attributes attr, Array<OElement> children);
    virtual ~Element();
    virtual void calcStyle(Style parentStyle) = 0;
    Array<RenderElement> render();

  protected:
    Style style;
    Attributes attr;
    Array<OElement> children;
};
DEFINE_REF(Element)
} // namespace UI
} // namespace Seele