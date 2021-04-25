#pragma once
#include "View.h"
namespace Seele
{
DECLARE_REF(Scene)
DECLARE_REF(CameraActor)
class SceneView : public View
{
public:
	SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
	~SceneView();
	virtual void beginFrame() override;
	PScene getScene() const { return scene; }
private:
	PScene scene;
	PCameraActor activeCamera;
	virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) override;
	virtual void mouseMoveCallback(double xPos, double yPos) override;
	virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) override;
	virtual void scrollCallback(double xOffset, double yOffset) override;
	virtual void fileCallback(int count, const char** paths) override;
};
DEFINE_REF(SceneView)
} // namespace Seele