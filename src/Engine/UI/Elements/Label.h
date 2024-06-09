#pragma once
#include "Element.h"

namespace Seele {
namespace UI {
class Label : public Element {
  public:
    Label();
    ~Label();

  private:
    std::string text;
};
} // namespace UI
} // namespace Seele
