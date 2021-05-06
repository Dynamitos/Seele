#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Asset/AssetRegistry.h"
#include "Graphics/RenderPass/RenderGraph.h"

using namespace Seele;

SceneRenderPath::SceneRenderPath(PScene scene, Gfx::PGraphics graphics, Gfx::PViewport target, PCameraActor source)
	: RenderPath(graphics, target)
	, scene(scene)
{
	renderGraph = new RenderGraph();
	depthPrepass = new DepthPrepass(renderGraph, scene, graphics, target, source);
	basePass = new BasePass(renderGraph, scene, graphics, target, source);
	renderGraph->setup();
}

SceneRenderPath::~SceneRenderPath()
{
}

void SceneRenderPath::setTargetScene(PScene newScene)
{
	scene = newScene;
}

void SceneRenderPath::init() 
{
    
}

void SceneRenderPath::beginFrame() 
{
	depthPrepass->beginFrame();
}

void SceneRenderPath::render() 
{
    depthPrepass->render();
}

void SceneRenderPath::endFrame() 
{
    depthPrepass->endFrame();
}
