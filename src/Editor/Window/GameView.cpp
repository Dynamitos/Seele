#include "GameView.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/KeyboardInput.h"
#include "Actor/CameraActor.h"

using namespace Seele;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo)
    : View(graphics, window, createInfo, "Game")
    , gameInterface("C:\\Users\\Dynamitos\\TrackClear\\bin\\TrackCleard.dll")
    , renderGraph(RenderGraphBuilder::build(
        DepthPrepass(graphics),
        LightCullingPass(graphics),
        BasePass(graphics),
#ifdef EDITOR
        DebugPass(graphics),
#endif
        SkyboxRenderPass(graphics)
    ))
{
    scene = new Scene(graphics);
    systemGraph = new SystemGraph();
    gameInterface.reloadGame()->setupScene(scene, systemGraph);
    renderGraph.updateViewport(viewport);
}

GameView::~GameView()
{
    
}

void GameView::beginUpdate()
{
}

void GameView::update()
{
    static auto startTime = std::chrono::high_resolution_clock::now();
    systemGraph->run(threadPool, updateTime);
    scene->update(updateTime);
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = (endTime - startTime);
    updateTime = duration.count();
    startTime = endTime;
}

void GameView::commitUpdate()
{
    depthPrepassData.staticDrawList = scene->getStaticMeshes();
    depthPrepassData.sceneDataBuffer = scene->getSceneDataBuffer();
    lightCullingData.lightEnv = scene->getLightBuffer();
    basePassData.staticDrawList = scene->getStaticMeshes();
    basePassData.sceneDataBuffer = scene->getSceneDataBuffer();
#ifdef EDITOR
    if(showDebug)
    {
        debugPassData.vertices = Seele::gDebugVertices;
    }
#endif
    Seele::gDebugVertices.clear();
}

void GameView::prepareRender()
{
#ifdef EDITOR
    renderGraph.updatePassData(depthPrepassData, lightCullingData, basePassData, debugPassData, skyboxData);
#else
    renderGraph.updatePassData(depthPrepassData, lightCullingData, basePassData, skyboxData);
#endif
}

void GameView::render()
{
    scene->view<Component::Camera>([&](Component::Camera& cam)
    {
        if(cam.mainCamera)
        {
            renderGraph.render(cam);
        }
    });
}

void GameView::keyCallback(KeyCode code, InputAction action, KeyModifier)
{
    scene->view<Component::KeyboardInput>([=](Component::KeyboardInput& input)
    {
        //if(code == KeyCode::KEY_R && action == InputAction::PRESS)
        //{
        //    auto cubeMesh = AssetRegistry::findMesh("cube");
        //    PEntity cube2 = new Entity(scene);
        //    Component::Transform& cube2Transform = cube2->attachComponent<Component::Transform>();
        //    cube2Transform.setPosition(Vector(0, 20, 0));
        //    cube2Transform.setRotation(Quaternion(Vector(0.f, 90.f, 90.f)));
        //    cube2Transform.setScale(Vector(2.f, 2.f, 2.f));
        //    cubeMesh->physicsMesh.type = Component::ColliderType::DYNAMIC;
        //    cube2->attachComponent<Component::Collider>(cubeMesh->physicsMesh);
        //    cube2->attachComponent<Component::StaticMesh>(cubeMesh);
        //    Component::RigidBody& physics2 = cube2->attachComponent<Component::RigidBody>();
        //    physics2.linearMomentum = Vector(0, -10, 0);
        //    physics2.angularMomentum = Vector(0, 0, 0);
        //}
        //
        //if(code == KeyCode::KEY_B && action == InputAction::PRESS)
        //{
        //    showDebug = !showDebug;
        //    debugPassData.vertices.clear();
        //}

        input.keys[code] = action != InputAction::RELEASE;
    });
}

void GameView::mouseMoveCallback(double xPos, double yPos)
{
    scene->view<Component::KeyboardInput>([=](Component::KeyboardInput& input)
    {
        input.mouseX = static_cast<float>(xPos);
        input.mouseY = static_cast<float>(yPos);
    });
}

void GameView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier)
{
    scene->view<Component::KeyboardInput>([=](Component::KeyboardInput& input)
    {
        if (button == MouseButton::MOUSE_BUTTON_1)
        {
            input.mouse1 = action != InputAction::RELEASE;
        }
        if (button == MouseButton::MOUSE_BUTTON_2)
        {
            input.mouse2 = action != InputAction::RELEASE;
        }
    });
}

void GameView::scrollCallback(double, double)
{
    
}

void GameView::fileCallback(int, const char**)
{
    
}
