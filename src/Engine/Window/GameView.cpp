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
#include "Graphics/RenderPass/ToneMappingPass.h"
#include "Graphics/RenderPass/VisibilityPass.h"
#include "System/CameraUpdater.h"
#include "System/LightGather.h"
#include "System/MeshUpdater.h"
#include "Window/Window.h"

using namespace Seele;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::filesystem::path dllPath)
    : View(graphics, window, createInfo, "Game"), graphics(graphics), scene(new Scene(graphics)), gameInterface(dllPath) {
    reloadGame();
    renderGraph.addPass(new CachedDepthPass(graphics, scene));
    renderGraph.addPass(new DepthCullingPass(graphics, scene));
    renderGraph.addPass(new VisibilityPass(graphics));
    renderGraph.addPass(new LightCullingPass(graphics, scene));
    renderGraph.addPass(new BasePass(graphics, scene));
    renderGraph.addPass(new ToneMappingPass(graphics));
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();
    if (graphics->supportRayTracing()) {
        rayTracingGraph.addPass(new RayTracingPass(graphics, scene));
        rayTracingGraph.addPass(new ToneMappingPass(graphics));
        rayTracingGraph.setViewport(viewport);
        rayTracingGraph.createRenderPass();
    }
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
    Component::Transform transform;
    scene->view<Component::Camera, Component::Transform>([&cam, &transform](Component::Camera& c, Component::Transform& t) {
        if (c.mainCamera) {
            cam = c;
            transform = t;
        }
    });
    if (getGlobals().useRayTracing && graphics->supportRayTracing()) {
        rayTracingGraph.render(cam, transform);
    } else {
        renderGraph.render(cam, transform);
    }
}

void GameView::applyArea(URect) {
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();
}

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
