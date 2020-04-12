#pragma once
#include "RenderPath.h"

namespace Seele
{
class SceneRenderPath : public RenderPath
{
public:
	SceneRenderPath(Gfx::PGraphics graphics);
	virtual ~SceneRenderPath();
	virtual void applyArea(Rect area) override;
	virtual void init() override;
	virtual void beginFrame() override;
	virtual void render() override;
	virtual void endFrame() override;
};
} // namespace Seele