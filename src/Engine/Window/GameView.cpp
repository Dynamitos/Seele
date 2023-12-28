#include "GameView.h"
#include "Graphics/Graphics.h"
#include "Window/Window.h"
#include "Component/KeyboardInput.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "System/LightGather.h"
#include "System/MeshUpdater.h"
#include "System/CameraUpdater.h"
#include "Graphics/Vulkan/Graphics.h"
#include "Graphics/Vulkan/Allocator.h"

using namespace Seele;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo &createInfo, std::string dllPath)
    : View(graphics, window, createInfo, "Game")
    , scene(new Scene(graphics))
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
    scene->getLightEnvironment()->commit();
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
}

void GameView::render()
{
    Component::Camera cam;
    scene->view<Component::Camera>([&cam](Component::Camera& c) {
        if (c.mainCamera) 
            cam = c;
    });
    renderGraph.render(cam);
}

void GameView::applyArea(URect rect)
{
    renderGraph.updateViewport(viewport);
}

void GameView::reloadGame()
{
    gameInterface.reload(AssetRegistry::getInstance());
    
    systemGraph = new SystemGraph();
    gameInterface.getGame()->setupScene(scene, systemGraph);
    System::OKeyboardInput keyInput = new System::KeyboardInput(scene);
    keyboardSystem = keyInput;
    systemGraph->addSystem(std::move(keyInput));
    systemGraph->addSystem(new System::LightGather(scene));
    systemGraph->addSystem(new System::MeshUpdater(scene));
    systemGraph->addSystem(new System::CameraUpdater(scene));
}

void GameView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier)
{
    if (code == KeyCode::KEY_P && action == InputAction::PRESS)
    {
        ((Vulkan::Graphics*)graphics.getHandle())->getAllocator()->print();
    }
    keyboardSystem->keyCallback(code, action, modifier);
}

void GameView::mouseMoveCallback(double xPos, double yPos)
{
    keyboardSystem->mouseCallback(xPos, yPos);
}

void GameView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier)
{
    keyboardSystem->mouseButtonCallback(button, action, modifier);
}

void GameView::scrollCallback(double, double)
{
}

void GameView::fileCallback(int, const char**)
{
}
