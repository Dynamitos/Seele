#pragma once
#include "View.h"
#include "UI/UIRenderPath.h"

namespace Seele
{
class InspectorView : public View
{
public:
    InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo);
    virtual ~InspectorView();
    virtual void beginFrame();
	virtual void render();
	virtual void endFrame();
	
private:
    PUIComponent root;
};
} // namespace Seele
