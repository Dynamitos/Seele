#pragma once
#include "Window/RenderPath.h"
#include "Element.h"

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
    Array<UI::PElement> rootElements;
};
DEFINE_REF(UIRenderPath);
} // namespace Seele

