#pragma once
#include "View.h"
namespace Seele
{
class SceneView : public View
{
public:
	SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
	~SceneView();
};
DEFINE_REF(SceneView);
} // namespace Seele