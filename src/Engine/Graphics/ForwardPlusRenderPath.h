#pragma once
#include "SceneRenderPath.h"

namespace Seele
{
class ForwardPlusRenderPath : public SceneRenderPath
{
public:
    ForwardPlusRenderPath(Gfx::PGraphics graphics, Gfx::PViewport target);
    virtual ~ForwardPlusRenderPath();
	virtual void beginFrame() override;
	virtual void render() override;
	virtual void endFrame() override;
private:
};
}