#pragma once
#include "MinimalEngine.h"
#include "RenderHierarchy.h"

namespace Seele
{
namespace UI
{
DECLARE_REF(Panel)
class System
{
public:
    System();
    virtual ~System();
private:
    PPanel rootPanel;
    Array<RenderHierarchyUpdate*> updates;
};
DEFINE_REF(System)
} // namespace UI
} // namespace Seele
