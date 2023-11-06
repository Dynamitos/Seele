#include "SceneView.h"
#include "Scene/Scene.h"
#include "Window/Window.h"
#include "Graphics/Mesh.h"
#include "Graphics/Graphics.h"
#include "Asset/MeshAsset.h"
#include "Asset/AssetRegistry.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"

using namespace Seele;
using namespace Seele::Editor;

SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
    : View(graphics, owner, createInfo, "SceneView")
    , scene(new Scene(graphics))
    , renderGraph(RenderGraphBuilder::build(
        DepthPrepass(graphics, scene),
        LightCullingPass(graphics, scene),
        BasePass(graphics, scene)
    ))
    , cameraSystem(createInfo.dimensions, Vector(0, 0, 10))
{

    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Ely\\Ely.fbx");
    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Cube\\cube.obj");
    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Plane\\plane.fbx");
    
    cameraSystem.update(viewportCamera, static_cast<float>(Gfx::getCurrentFrameDelta()));
    
    renderGraph.updateViewport(viewport);
}

SceneView::~SceneView()
{
}

void SceneView::beginUpdate() 
{
    //co_return;
}

void SceneView::update() 
{
    cameraSystem.update(viewportCamera, static_cast<float>(Gfx::getCurrentFrameDelta()));
    //co_return;
}

void SceneView::commitUpdate() 
{
}

void SceneView::prepareRender() 
{
}

void SceneView::render() 
{
    renderGraph.render(viewportCamera);
}

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier)
{
    cameraSystem.keyCallback(code, action);
}

void SceneView::mouseMoveCallback(double xPos, double yPos) 
{
    cameraSystem.mouseMoveCallback(xPos, yPos);
}

void SceneView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier) 
{
    cameraSystem.mouseButtonCallback(button, action);
}

void SceneView::scrollCallback(double, double) 
{
}

void SceneView::fileCallback(int, const char**) 
{
    
}
