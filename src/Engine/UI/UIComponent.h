#pragma once
#include "MinimalEngine.h"
namespace Seele
{
DECLARE_REF(UIRenderPath)
class UIComponent
{
public:
    UIComponent(PUIRenderPath renderer);
    virtual ~UIComponent();
private:
    PUIRenderPath renderer;
};
DEFINE_REF(UIComponent);
} // namespace Seele
