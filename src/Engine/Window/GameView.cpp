#include "GameView.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Component/KeyboardInput.h"
#include "Graphics/Graphics.h"
#include "Graphics/Query.h"
#include "Graphics/RenderPass/BasePass.h"
#include "Graphics/RenderPass/CachedDepthPass.h"
#include "Graphics/RenderPass/DepthCullingPass.h"
#include "Graphics/RenderPass/LightCullingPass.h"
#include "Graphics/RenderPass/RayTracingPass.h"
#include "Graphics/RenderPass/RenderGraphResources.h"
#include "Graphics/RenderPass/VisibilityPass.h"
#include "System/CameraUpdater.h"
#include "System/LightGather.h"
#include "System/MeshUpdater.h"
#include "Window/Window.h"
#include <fstream>
#include <thread>

using namespace Seele;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath)
    : View(graphics, window, createInfo, "Game"), scene(new Scene(graphics)), gameInterface(dllPath) {
    reloadGame();
    renderGraph.addPass(new CachedDepthPass(graphics, scene));
    renderGraph.addPass(new DepthCullingPass(graphics, scene));
    renderGraph.addPass(new VisibilityPass(graphics, scene));
    renderGraph.addPass(new LightCullingPass(graphics, scene));
    renderGraph.addPass(new BasePass(graphics, scene));
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();

    rayTracingGraph.addPass(new RayTracingPass(graphics, scene));
    rayTracingGraph.setViewport(viewport);
    rayTracingGraph.createRenderPass();
}

GameView::~GameView() {}

void GameView::beginUpdate() {}

void GameView::update() {
    static auto startTime = std::chrono::high_resolution_clock::now();
    for (VertexData* vd : VertexData::getList()) {
        vd->resetMeshData();
    }
    systemGraph->run(updateTime);
    scene->update(updateTime);
    for (VertexData* vd : VertexData::getList()) {
        vd->createDescriptors();
    }
    scene->getLightEnvironment()->commit();
    auto endTime = std::chrono::high_resolution_clock::now();
    std::chrono::duration<float> duration = (endTime - startTime);
    updateTime = duration.count();
    startTime = endTime;
}

void GameView::commitUpdate() {}

void GameView::prepareRender() {}

void GameView::render() {
    Component::Camera cam;
    scene->view<Component::Camera>([&cam](Component::Camera& c) {
        if (c.mainCamera)
            cam = c;
    });
    if (getGlobals().useRayTracing) {
        rayTracingGraph.render(cam);
    } else {
        renderGraph.render(cam);
    }
}

void GameView::applyArea(URect) { renderGraph.setViewport(viewport); }

void GameView::reloadGame() {
    gameInterface.reload();

    systemGraph = new SystemGraph();
    gameInterface.getGame()->setupScene(scene, systemGraph);
    System::OKeyboardInput keyInput = new System::KeyboardInput(scene);
    keyboardSystem = keyInput;
    systemGraph->addSystem(std::move(keyInput));
    systemGraph->addSystem(new System::LightGather(scene));
    systemGraph->addSystem(new System::MeshUpdater(scene));
    systemGraph->addSystem(new System::CameraUpdater(scene));
}

void GameView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) { keyboardSystem->keyCallback(code, action, modifier); }

void GameView::mouseMoveCallback(double xPos, double yPos) { keyboardSystem->mouseCallback(xPos, yPos); }

void GameView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) {
    keyboardSystem->mouseButtonCallback(button, action, modifier);
}

void GameView::scrollCallback(double xScroll, double yScroll) { keyboardSystem->scrollCallback(xScroll, yScroll); }

void GameView::fileCallback(int, const char**) {}
