#pragma once
#include "Element.h"

namespace Seele {
namespace UI {
class System {
  public:
    System(OElement rootElement) : rootElement(std::move(rootElement)) {}
    ~System() {}
    void style() { rootElement->calcStyle(UI::Style()); }
    void layout(UVector2 viewport) { rootElement->layout(viewport); }
    Array<UIRender> render() { return rootElement->render(Vector2(0, 0), 1); }

  private:
    OElement rootElement;
};
DEFINE_REF(System)
} // namespace UI
} // namespace Seele