#pragma once
#include "Window/View.h"
#include "Scene/Scene.h"
#include "Graphics/RenderPass/DepthPrepass.h"
#include "Graphics/RenderPass/LightCullingPass.h"
#include "Graphics/RenderPass/BasePass.h"
#include "Graphics/RenderPass/SkyboxRenderPass.h"
#ifdef WIN32
#include "Platform/Windows/GameInterface.h" // TODO
#else
#include "Platform/Linux/GameInterface.h"
#endif

namespace Seele
{
class GameView : public View
{
public:
    GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo, std::string dllPath);
    virtual ~GameView();
	virtual void beginUpdate() override;
	virtual void update() override;
	virtual void commitUpdate() override;

	virtual void prepareRender() override;
	virtual void render() override;

	void reloadGame();
private:
	OScene scene;
	OEntity camera;
	GameInterface gameInterface;
    RenderGraph<
        DepthPrepass
		//LightCullingPass,
        //BasePass,
		//SkyboxRenderPass
		> renderGraph;

	PSystemGraph systemGraph;
	dp::thread_pool<> threadPool;
	float updateTime = 0;

	virtual void keyCallback(Seele::KeyCode code, Seele::InputAction action, Seele::KeyModifier modifier) override;
	virtual void mouseMoveCallback(double xPos, double yPos) override;
	virtual void mouseButtonCallback(Seele::MouseButton button, Seele::InputAction action, Seele::KeyModifier modifier) override;
	virtual void scrollCallback(double xOffset, double yOffset) override;
	virtual void fileCallback(int count, const char** paths) override;
};
DEFINE_REF(GameView)
} // namespace Seele
