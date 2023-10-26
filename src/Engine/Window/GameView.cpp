#include "GameView.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/KeyboardInput.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"

using namespace Seele;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo, std::string dllPath)
    : View(graphics, window, createInfo, "Game")
    , gameInterface(dllPath)
    , renderGraph(RenderGraphBuilder::build(
        DepthPrepass(graphics, scene),
        LightCullingPass(graphics, scene),
        BasePass(graphics, scene),
        SkyboxRenderPass(graphics, scene)
    ))
{
    reloadGame();
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
    for (VertexData* vd : VertexData::getList())
    {
        vd->resetMeshData();
    }
    systemGraph->run(threadPool, updateTime);
    scene->update(updateTime);
    for (VertexData* vd : VertexData::getList())
    {
        vd->createDescriptors();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = (endTime - startTime);
    updateTime = duration.count();
    startTime = endTime;
}

void GameView::commitUpdate()
{

}

void GameView::prepareRender()
{
    renderGraph.updatePassData(depthPrepassData, lightCullingData, basePassData, skyboxData);
}

void GameView::render()
{
    renderGraph.render(camera->accessComponent<Component::Camera>());
}

void GameView::reloadGame()
{
    scene = new Scene(graphics);
    gameInterface.reload(AssetRegistry::getInstance());
    
    systemGraph = new SystemGraph();
    gameInterface.getGame()->setupScene(scene, systemGraph);
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
        if(code == KeyCode::KEY_R && action == InputAction::PRESS)
        {
            reloadGame();
        }

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
