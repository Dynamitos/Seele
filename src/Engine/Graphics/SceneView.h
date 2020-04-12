#pragma once
#include "View.h"
namespace Seele
{
class SceneView : public View
{
public:
	SceneView(Gfx::PGraphics graphics);
	~SceneView();
};
} // namespace Seele