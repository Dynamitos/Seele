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
    PPrimitiveComponent ayaka = new PrimitiveComponent(AssetRegistry::findMesh("Ayaka"));
    ayaka->setRelativeLocation(Vector(0, 0, 0));
    ayaka->setRelativeScale(Vector(10, 10, 10));
    scene->addPrimitiveComponent(ayaka);

    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Ely\\Ely.fbx");
    //AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Cube\\cube.obj");
    AssetRegistry::importFile("D:\\Private\\Programming\\Unreal Engine\\Assets\\Plane\\plane.fbx");
    PPrimitiveComponent plane = new PrimitiveComponent(AssetRegistry::findMesh("plane"));
    plane->setRelativeScale(Vector(100, 100, 100));
    scene->addPrimitiveComponent(plane);

    for(uint32 i = 0; i < 10; ++i)
    {
        PMyComponent myComp = new MyComponent();
        PMyOtherComponent myOtherComp = new MyOtherComponent();
        PActor actor = new Actor();
        actor->setRootComponent(myComp);
        myComp->addChildComponent(myOtherComp);
        scene->addActor(actor);
    }
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

void SceneView::beginUpdate() 
{
    scene->beginUpdate(Gfx::currentFrameDelta);
    //co_return;
}

void SceneView::update() 
{
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
    depthPrepass.updateViewFrame(depthPrepassData);
    lightCullingPass.updateViewFrame(lightCullingPassData);
    basePass.updateViewFrame(basePassData);
}

void SceneView::render() 
{
    depthPrepass.beginFrame();
    lightCullingPass.beginFrame();
    basePass.beginFrame();
    depthPrepass.render();
    lightCullingPass.render();
    basePass.render();
    depthPrepass.endFrame();
    lightCullingPass.endFrame();
    basePass.endFrame();
}

static float cameraSpeed = 1;

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier mod)
{
    if((KeyModifierFlags)mod & (KeyModifierFlags)KeyModifier::MOD_SHIFT)
    {
        cameraSpeed = 5;
    }
    else
    {
        cameraSpeed = 1;
    }
    if(action != InputAction::RELEASE)
    {
        if(code == KeyCode::KEY_W)
        {
            activeCamera->getCameraComponent()->moveX(1);
        }
        if(code == KeyCode::KEY_S)
        {
            activeCamera->getCameraComponent()->moveX(-1);
        }
        if(code == KeyCode::KEY_A)
        {
            activeCamera->getCameraComponent()->moveY(1);
        }
        if(code == KeyCode::KEY_D)
        {
            activeCamera->getCameraComponent()->moveY(-1);
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
    if(button == MouseButton::MOUSE_BUTTON_2 && action != InputAction::RELEASE)
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
    activeCamera->getCameraComponent()->mouseScroll(static_cast<float>(yOffset));
}

void SceneView::fileCallback(int, const char**) 
{
    
}
