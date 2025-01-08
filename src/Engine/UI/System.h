#pragma once
#include "Element.h"

namespace Seele {
namespace UI {
struct RenderElement {
    // position relative to target viewport in pixels
    UVector2 position;
    // size relative to target viewport in pixels
    UVector2 size;
    // background color of area
    Vector backgroundColor;
    // text to render
    std::string text;
    // font size in pixels
    uint32 fontSize;
    // font family
    PFontAsset fontFamily;
};
class System {
  public:
    System(OElement rootElement) : rootElement(std::move(rootElement)) {}
    ~System();
    Array<RenderElement> render(UVector2 viewport) {
    }

  private:
    OElement rootElement;
};
DEFINE_REF(System)
} // namespace UI
} // namespace Seele