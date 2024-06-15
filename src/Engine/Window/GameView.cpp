#include "GameView.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Component/KeyboardInput.h"
#include "Graphics/Graphics.h"
#include "Graphics/Query.h"
#include "Graphics/RenderPass/BasePass.h"
#include "Graphics/RenderPass/CachedDepthPass.h"
#include "Graphics/RenderPass/DebugPass.h"
#include "Graphics/RenderPass/DepthCullingPass.h"
#include "Graphics/RenderPass/LightCullingPass.h"
#include "Graphics/RenderPass/RenderGraphResources.h"
#include "Graphics/RenderPass/SkyboxRenderPass.h"
#include "Graphics/RenderPass/VisibilityPass.h"
#include "System/CameraUpdater.h"
#include "System/LightGather.h"
#include "System/MeshUpdater.h"
#include "Window/Window.h"

using namespace Seele;

bool usePositionOnly = false;
bool useDepthCulling = false;
bool useLightCulling = false;

GameView::GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath)
    : View(graphics, window, createInfo, "Game"), scene(new Scene(graphics)), gameInterface(dllPath) {
    reloadGame();
    renderGraph.addPass(new CachedDepthPass(graphics, scene));
    renderGraph.addPass(new DepthCullingPass(graphics, scene));
    renderGraph.addPass(new VisibilityPass(graphics, scene));
    renderGraph.addPass(new LightCullingPass(graphics, scene));
    renderGraph.addPass(new BasePass(graphics, scene));
    renderGraph.addPass(new DebugPass(graphics, scene));
    //    renderGraph.addPass(new SkyboxRenderPass(graphics, scene));
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();
    queryThread = std::thread([&]() {
        PRenderGraphResources res = renderGraph.getResources();
        Gfx::PPipelineStatisticsQuery cachedQuery = res->requestQuery("CACHED_QUERY");
        Gfx::PPipelineStatisticsQuery depthQuery = res->requestQuery("DEPTH_QUERY");
        Gfx::PPipelineStatisticsQuery baseQuery = res->requestQuery("BASEPASS_QUERY");
        Gfx::PPipelineStatisticsQuery lightCullQuery = res->requestQuery("LIGHTCULL_QUERY");
        Gfx::PPipelineStatisticsQuery visibilityQuery = res->requestQuery("VISIBILITY_QUERY");
        while (true) {
            auto cachedResults = cachedQuery->getResults();
            auto depthResults = depthQuery->getResults();
            auto baseResults = baseQuery->getResults();
            auto lightCullResults = lightCullQuery->getResults();
            auto visiblityResults = visibilityQuery->getResults();
            std::cout << "Pipeline Stats: "
                      << cachedResults.meshShaderInvocations + depthResults.meshShaderInvocations + baseResults.meshShaderInvocations +
                             lightCullResults.meshShaderInvocations + visiblityResults.meshShaderInvocations
                      << std::endl;
        }
    });
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
    renderGraph.render(cam);
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

void GameView::keyCallback(KeyCode code, InputAction action, KeyModifier modifier) {
    keyboardSystem->keyCallback(code, action, modifier);
    if (code == KeyCode::KEY_P && action == InputAction::RELEASE) {
        usePositionOnly = !usePositionOnly;
        std::cout << "Use Pos only " << usePositionOnly << std::endl;
    }
    if (code == KeyCode::KEY_O && action == InputAction::RELEASE) {
        useDepthCulling = !useDepthCulling;
        std::cout << "Use Depth Culling " << useDepthCulling << std::endl;
    }
    if (code == KeyCode::KEY_L && action == InputAction::RELEASE) {
        useLightCulling = !useLightCulling;
        std::cout << "Use Light Culling " << useLightCulling << std::endl;
    }
}

void GameView::mouseMoveCallback(double xPos, double yPos) { keyboardSystem->mouseCallback(xPos, yPos); }

void GameView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) {
    keyboardSystem->mouseButtonCallback(button, action, modifier);
}

void GameView::scrollCallback(double xScroll, double yScroll) { keyboardSystem->scrollCallback(xScroll, yScroll); }

void GameView::fileCallback(int, const char**) {}
