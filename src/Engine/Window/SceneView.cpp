#include "SceneView.h"
#include "Scene/Scene.h"
#include "Window.h"
#include "Graphics/Graphics.h"
#include "Asset/MeshAsset.h"
#include "Asset/AssetRegistry.h"
#include "Scene/Actor/CameraActor.h"
#include "Scene/Components/CameraComponent.h"
#include "Scene/Components/MyComponent.h"
#include "Scene/Components/MyOtherComponent.h"

using namespace Seele;

Seele::SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo)
    : View(graphics, owner, createInfo, "SceneView")
    , activeCamera(new CameraActor())
    , depthPrepass(DepthPrepass(graphics, viewport, activeCamera))
    , lightCullingPass(LightCullingPass(graphics, viewport, activeCamera))
    , basePass(BasePass(graphics, viewport, activeCamera))
{
    scene = new Scene(graphics);
    scene->addActor(activeCamera);
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Body_Diffuse.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Body_Lightmap.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Face_Diffuse.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Hair_Diffuse.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Sword_Ayaka_Tex_Hair_Lightmap.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Avatar_Girl_Tex_FaceLightmap.png");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ayaka\\Ayaka.fbx");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Ely\\Ely.fbx");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Cube\\cube.obj");
    AssetRegistry::importFile("C:\\Users\\Dynamitos\\TestSeeleProject\\Assets\\Plane\\plane.fbx");
 
    PPrimitiveComponent ayaka = new PrimitiveComponent(AssetRegistry::findMesh("Ayaka"));
    ayaka->addWorldTranslation(Vector(0, 0, 0));
    ayaka->setWorldScale(Vector(10, 10, 10));
    scene->addPrimitiveComponent(ayaka);

    PPrimitiveComponent plane = new PrimitiveComponent(AssetRegistry::findMesh("plane"));
    plane->setWorldScale(Vector(100, 100, 100));
    scene->addPrimitiveComponent(plane);

    PMyComponent myComp = new MyComponent();
    PMyOtherComponent myOtherComp = new MyOtherComponent();
    PActor actor = new Actor();
    actor->setRootComponent(myComp);
    myComp->addChildComponent(myOtherComp);
    scene->addActor(actor);
    scene->start();
    
    PRenderGraphResources resources = new RenderGraphResources();
    depthPrepass.setResources(resources);
    lightCullingPass.setResources(resources);
    basePass.setResources(resources);

    depthPrepass.publishOutputs();
    lightCullingPass.publishOutputs();
    basePass.publishOutputs();

    depthPrepass.createRenderPass();
    lightCullingPass.createRenderPass();
    basePass.createRenderPass();
}

Seele::SceneView::~SceneView()
{
}

Job SceneView::beginUpdate() 
{
    co_await scene->beginUpdate(Gfx::currentFrameDelta);
    co_return;
}

Job SceneView::update() 
{
    co_await scene->commitUpdate();
    co_return;
}

void SceneView::commitUpdate() 
{
    depthPrepassData.staticDrawList = scene->getStaticMeshes();
    lightCullingPassData.lightEnv = scene->getLightBuffer();
    basePassData.staticDrawList = scene->getStaticMeshes();
}

void SceneView::prepareRender() 
{
    depthPrepass.updateViewFrame(depthPrepassData);
    lightCullingPass.updateViewFrame(lightCullingPassData);
    basePass.updateViewFrame(basePassData);
}

MainJob SceneView::render() 
{
    co_await depthPrepass.beginFrame();
    co_await lightCullingPass.beginFrame();
    co_await basePass.beginFrame();
    co_await depthPrepass.render();
    co_await lightCullingPass.render();
    co_await basePass.render();
    co_await depthPrepass.endFrame();
    co_await lightCullingPass.endFrame();
    co_await basePass.endFrame();
    renderFinishedEvent.raise();
    co_return;
}

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier)
{
    if(action != InputAction::RELEASE)
    {
        if(code == KeyCode::KEY_W)
        {
            activeCamera->getCameraComponent()->moveOrigin(1);
        }
        if(code == KeyCode::KEY_S)
        {
            activeCamera->getCameraComponent()->moveOrigin(-1);
        }
    }
}

static bool mouseDown = false;

void SceneView::mouseMoveCallback(double xPos, double yPos) 
{
    static double prevXPos = 0.0f, prevYPos = 0.0f;
    double deltaX = prevXPos - xPos;
    double deltaY = prevYPos - yPos;
    prevXPos = xPos;
    prevYPos = yPos;
    if(mouseDown)
    {
        activeCamera->getCameraComponent()->mouseMove((float)deltaX, (float)deltaY);
    }
}

void SceneView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier) 
{
    if(button == MouseButton::MOUSE_BUTTON_1 && action != InputAction::RELEASE)
    {
        mouseDown = true;
    }
    else
    {
        mouseDown = false;
    }
}

void SceneView::scrollCallback(double, double yOffset) 
{
    activeCamera->getCameraComponent()->mouseScroll(yOffset);
}

void SceneView::fileCallback(int, const char**) 
{
    
}
