#pragma once
#include "RenderPath.h"

namespace Seele
{
class SceneRenderPath : public RenderPath
{
public:
	SceneRenderPath(Gfx::PGraphics graphics, Gfx::PViewport target);
	virtual ~SceneRenderPath();
	virtual void beginFrame() = 0;
	virtual void render() = 0;
	virtual void endFrame() = 0;
};
} // namespace Seele