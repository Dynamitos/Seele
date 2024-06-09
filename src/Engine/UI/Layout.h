#pragma once
#include "MinimalEngine.h"

namespace Seele {
namespace UI {
DECLARE_REF(Element);
class Layout {
  public:
    Layout(PElement element);
    virtual ~Layout();
    virtual void apply() = 0;

  protected:
    PElement element;
};
} // namespace UI
} // namespace Seele
