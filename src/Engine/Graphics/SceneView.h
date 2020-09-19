#pragma once
#include "View.h"
namespace Seele
{
DECLARE_REF(Scene);
class SceneView : public View
{
public:
	SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
	~SceneView();
	PScene getScene() const { return scene; }
private:
	PScene scene;
};
DEFINE_REF(SceneView);
} // namespace Seele