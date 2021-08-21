#pragma once
#include "MinimalEngine.h"

namespace Seele
{
namespace UI
{
//Element defines any part of the UI
DECLARE_REF(Element)
class Element
{
public:
    Element();
    virtual ~Element();
    void addElement(PElement element);
    const Array<PElement> getChildren() const;
    void clear();
    void remove(PElement element);
    void setEnabled(bool newEnabled);
    bool isEnabled() const;
protected:
    bool enabled;
    Array<PElement> children;
};
DEFINE_REF(Element)
} // namespace UI
} // namespace Seele
