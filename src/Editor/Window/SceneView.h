#pragma once
#include <thread_pool/thread_pool.h>
#include "Window/View.h"
#include "Graphics/RenderPass/DepthPrepass.h"
#include "Graphics/RenderPass/LightCullingPass.h"
#include "Graphics/RenderPass/BasePass.h"
#include "ViewportControl.h"

namespace Seele
{
DECLARE_REF(Scene)
namespace Editor
{

class SceneView : public View
{
public:
	SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
	~SceneView();

	virtual void beginUpdate() override;
	virtual void update() override;
	virtual void commitUpdate() override;

	virtual void prepareRender() override;
	virtual void render() override;

	PScene getScene() const { return scene; }
private:
	PScene scene;
	Component::Camera viewportCamera;
	
	RenderGraph<
		DepthPrepass,
		LightCullingPass,
		BasePass> renderGraph;

	DepthPrepassData depthPrepassData;
	LightCullingPassData lightCullingPassData;
	BasePassData basePassData;

	dp::thread_pool<> pool;
	ViewportControl cameraSystem;

	virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) override;
	virtual void mouseMoveCallback(double xPos, double yPos) override;
	virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) override;
	virtual void scrollCallback(double xOffset, double yOffset) override;
	virtual void fileCallback(int count, const char** paths) override;
};
DEFINE_REF(SceneView)
} // namespace Editor
} // namespace Seele