#pragma once
#include "Layout.h"

namespace Seele {
namespace UI {
class VerticalLayout : Layout {
  public:
    VerticalLayout(PElement element);
    ~VerticalLayout();
    virtual void apply() override;

  private:
};
} // namespace UI
} // namespace Seele