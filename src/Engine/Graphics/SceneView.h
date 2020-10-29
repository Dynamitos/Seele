#pragma once
#include "View.h"
namespace Seele
{
DECLARE_REF(Scene);
DECLARE_REF(CameraActor);
class SceneView : public View
{
public:
	SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
	~SceneView();
	PScene getScene() const { return scene; }
private:
	PScene scene;
	PCameraActor activeCamera;
	virtual void keyCallback(KeyCode code, KeyAction action, KeyModifier modifier) override;
};
DEFINE_REF(SceneView);
} // namespace Seele