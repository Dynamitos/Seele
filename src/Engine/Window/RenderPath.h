#pragma once
#include "Graphics/Graphics.h"
namespace Seele
{
//A renderpath is a general Renderer for a view.
class RenderPath
{
public:
	RenderPath(Gfx::PGraphics graphics, Gfx::PViewport target);
	virtual ~RenderPath();
	virtual void applyArea(URect area);
	virtual void init() = 0;
	virtual void beginFrame() = 0;
	virtual void render() = 0;
	virtual void endFrame() = 0;

protected:
	Gfx::PGraphics graphics;
	Gfx::PViewport target;
};
DEFINE_REF(RenderPath);
} // namespace Seele