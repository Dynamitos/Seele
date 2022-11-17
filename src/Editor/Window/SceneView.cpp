#include "SceneView.h"
#include "Scene/Scene.h"
#include "Window/Window.h"
#include "Graphics/Mesh.h"
#include "Graphics/Graphics.h"
#include "Asset/MeshAsset.h"
#include "Asset/AssetRegistry.h"
#include "Actor/CameraActor.h"
#include "Component/Camera.h"
#include "Component/StaticMesh.h"

using namespace Seele;
using namespace Seele::Editor;

SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
    : View(graphics, owner, createInfo, "SceneView")
    , scene(new Scene(graphics))
    , renderGraph(RenderGraphBuilder::build(
        DepthPrepass(graphics),
        LightCullingPass(graphics),
        BasePass(graphics)
    ))
    , cameraSystem(createInfo.dimensions, Math::Vector(0, 0, 10))
{
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Body_Diffuse.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Body_Lightmap.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Face_Diffuse.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Hair_Diffuse.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Hair_Lightmap.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Tex_FaceLightmap.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Ayaka.fbx");
    auto meshAsset = AssetRegistry::findMesh("Ayaka");
    for(auto mesh : meshAsset->getMeshes())
    {
        PActor actor = new Actor(scene);
        actor->attachComponent<Component::StaticMesh>(mesh->vertexInput, mesh->indexBuffer, mesh->referencedMaterial);
    }

    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Ely\\Ely.fbx");
    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Cube\\cube.obj");
    AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Plane\\plane.fbx");
    
    cameraSystem.update(viewportCamera, static_cast<float>(Gfx::currentFrameDelta));
    
    renderGraph.updateViewport(viewport);
}

SceneView::~SceneView()
{
}

void SceneView::beginUpdate() 
{
    scene->beginUpdate(Gfx::currentFrameDelta);
    //co_return;
}

void SceneView::update() 
{
    cameraSystem.update(viewportCamera, static_cast<float>(Gfx::currentFrameDelta));
    scene->commitUpdate();
    //co_return;
}

void SceneView::commitUpdate() 
{
    depthPrepassData.staticDrawList = scene->getStaticMeshes();
    lightCullingPassData.lightEnv = scene->getLightBuffer();
    basePassData.staticDrawList = scene->getStaticMeshes();
}

void SceneView::prepareRender() 
{
    renderGraph.updatePassData(depthPrepassData, lightCullingPassData, basePassData);
}

void SceneView::render() 
{
    renderGraph.render(viewportCamera);
}

static float cameraSpeed = 1;

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier)
{
    cameraSystem.keyCallback(code, action);
}

static bool mouseDown = false;

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
