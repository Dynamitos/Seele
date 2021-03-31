#pragma once
#include "RenderPath.h"
#include "Graphics/RenderPass/BasePass.h"

namespace Seele
{
DECLARE_REF(Scene);
DECLARE_REF(CameraActor);
class SceneRenderPath : public RenderPath
{
public:
	SceneRenderPath(PScene scene, Gfx::PGraphics graphics, Gfx::PViewport target, PCameraActor source);
	virtual ~SceneRenderPath();
	void setTargetScene(PScene scene);
	virtual void init();
	virtual void beginFrame();
	virtual void render();
	virtual void endFrame();

protected:
	PScene scene;
	UPBasePass basePass;
};
} // namespace Seele