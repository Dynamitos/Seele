#pragma once
#include "Elements/Element.h"

namespace Seele
{
namespace UI
{
class Layout
{
public:
    Layout(PElement element);
    virtual ~Layout();
    virtual void apply() = 0;
protected:
    PElement element;
};
} // namespace UI
} // namespace Seele
