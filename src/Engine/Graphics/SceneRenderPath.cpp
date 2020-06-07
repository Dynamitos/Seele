#include "SceneRenderPath.h"
#include "Scene/Scene.h"
#include "Material/Material.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

SceneRenderPath::SceneRenderPath(Gfx::PGraphics graphics, Gfx::PViewport target)
	: RenderPath(graphics, target)
{
	scene = new Scene();
	PMeshAsset asset = AssetRegistry::findMesh("Unbenannt");
	PActor rootActor = new Actor();
	PPrimitiveComponent primitiveComponent = new PrimitiveComponent();
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
    
}

void SceneRenderPath::endFrame() 
{
    
}
