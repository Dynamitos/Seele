#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

SceneRenderPath::SceneRenderPath(PScene scene, Gfx::PGraphics graphics, Gfx::PViewport target)
	: RenderPath(graphics, target)
	, scene(scene)
{
	basePass = new BasePass(scene, graphics, target);
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
    
}

void SceneRenderPath::render() 
{
    basePass->render();
}

void SceneRenderPath::endFrame() 
{
    
}
