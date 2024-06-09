#pragma once
#include "Layout.h"

namespace Seele {
namespace UI {
class HorizontalLayout : Layout {
  public:
    HorizontalLayout(PElement element);
    ~HorizontalLayout();
    virtual void apply() override;

  private:
};
} // namespace UI
} // namespace Seele