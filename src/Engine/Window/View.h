#pragma once
#include "Graphics/RenderPass/RenderGraph.h"

namespace Seele
{
DECLARE_REF(Window)

// A ViewFrame is the render relevant data of a View
class ViewFrame
{
public:
protected:
};
DEFINE_REF(ViewFrame)

// A view is a part of the window, which can be anything from a scene viewport to an inspector
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
	UPViewFrame currentFrame;
	Gfx::PGraphics graphics;
	Gfx::PViewport viewport;
	PWindow owner;
	PRenderGraph renderGraph;

	virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) = 0;
	virtual void mouseMoveCallback(double xPos, double yPos) = 0;
	virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) = 0;
	virtual void scrollCallback(double xOffset, double yOffset) = 0;
	virtual void fileCallback(int count, const char** paths) = 0;
	friend class Window;
};

DEFINE_REF(View)
} // namespace Seele