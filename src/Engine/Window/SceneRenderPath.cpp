#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

SceneRenderPath::SceneRenderPath(PScene scene, Gfx::PGraphics graphics, Gfx::PViewport target, PCameraActor source)
	: RenderPath(graphics, target)
	, scene(scene)
{
	basePass = new BasePass(scene, graphics, target, source);
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
    basePass->beginFrame();
}

void SceneRenderPath::render() 
{
    basePass->render();
}

void SceneRenderPath::endFrame() 
{
    basePass->endFrame();
}
