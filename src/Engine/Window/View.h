#pragma once
#include "RenderPath.h"
namespace Seele
{
DECLARE_REF(Window)
// A view is a part of the window, which can be anything from a viewport to an editor
class View
{
public:
	View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo);
	virtual ~View();
	virtual void beginFrame();
	virtual void render();
	virtual void endFrame();
	void applyArea(URect area);
	void setFocused();

protected:
	Gfx::PGraphics graphics;
	Gfx::PViewport viewport;
	PWindow owner;
	PRenderPath renderer;

	virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) = 0;
	virtual void mouseMoveCallback(double xPos, double yPos) = 0;
	virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) = 0;
	virtual void scrollCallback(double xOffset, double yOffset) = 0;
	virtual void fileCallback(int count, const char** paths) = 0;
	friend class Window;
};

DEFINE_REF(View)
} // namespace Seele