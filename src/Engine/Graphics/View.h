#pragma once
#include "RenderPath.h"
namespace Seele
{
DECLARE_REF(Window);
// A view is a part of the window, which can be anything from a viewport to an editor
class View
{
public:
	View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo);
	virtual ~View();
	void beginFrame();
	void render();
	void endFrame();
	void applyArea(URect area);
	void setFocused();

protected:
	Gfx::PGraphics graphics;
	Gfx::PViewport viewport;
	PWindow owner;
	PRenderPath renderer;

	virtual void keyCallback(KeyCode code, KeyAction action, KeyModifier modifier) = 0;
	friend class Window;
};

DEFINE_REF(View)
} // namespace Seele