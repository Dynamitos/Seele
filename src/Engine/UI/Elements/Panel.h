#pragma once
#include "Element.h"

namespace Seele {
namespace UI {
DECLARE_REF(Layout)
class Panel : Element {
  public:
    Panel();
    virtual ~Panel();

  private:
    PLayout activeLayout;
};

DEFINE_REF(Panel);
} // namespace UI
} // namespace Seele
