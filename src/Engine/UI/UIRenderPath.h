#pragma once
#include "Window/RenderPath.h"
#include "UIComponent.h"

namespace Seele
{
class UIRenderPath : public RenderPath
{
public:
    UIRenderPath();
    virtual ~UIRenderPath();
    virtual void beginFrame();
    virtual void render();
    virtual void endFrame();
private:
};
DEFINE_REF(UIRenderPath);
} // namespace Seele

