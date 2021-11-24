#pragma once
#include "Graphics/RenderPass/RenderGraph.h"
#include "ThreadPool.h"

namespace Seele
{
DECLARE_REF(Window)
// A view is a part of the window, which can be anything from a scene viewport to an inspector
class View
{
public:
	View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo, std::string name);
	virtual ~View();
	
	// These are called from the view thread, and handle updating game data
	virtual void beginUpdate() {}
	virtual void update() {}
	// End frame is called with a lock, so it is safe to write to shared memory
	virtual void commitUpdate() {}

	// These are called from the render thread
	// prepare render is also locked, so reading from shared memory is also safe
	virtual void prepareRender() {}
	virtual Job render() { co_return; }
	void applyArea(URect area);
	void setFocused();
	Event renderFinished() { return renderFinishedEvent; }

protected:
	Gfx::PGraphics graphics;
	Gfx::PViewport viewport;
	PWindow owner;
	std::string name;
	Event renderFinishedEvent;
	
	virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) = 0;
	virtual void mouseMoveCallback(double xPos, double yPos) = 0;
	virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) = 0;
	virtual void scrollCallback(double xOffset, double yOffset) = 0;
	virtual void fileCallback(int count, const char** paths) = 0;
	friend class Window;
};

DEFINE_REF(View)
} // namespace Seele