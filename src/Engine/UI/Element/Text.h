#pragma once
#include "UI/Element.h"
#include "UI/Style/Class.h"

namespace Seele {
namespace UI {
class Text : public Element {
  public:
    Text(std::string text) : Element({}, {}), text(text) {}
    virtual void calcStyle(Style parentStyle) override { style = parentStyle; }

  private:
    std::string text;
};
} // namespace UI
} // namespace Seele