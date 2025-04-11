#include "SceneView.h"
#include "Actor/CameraActor.h"
#include "Asset/AssetRegistry.h"
#include "Asset/MeshAsset.h"
#include "Component/Camera.h"
#include "Component/Mesh.h"
#include "Graphics/Graphics.h"
#include "Graphics/Mesh.h"
#include "Scene/Scene.h"
#include "Window/Window.h"
#include "Graphics/RenderPass/BasePass.h"
#include "Graphics/RenderPass/DepthCullingPass.h"
#include "Graphics/RenderPass/LightCullingPass.h"

using namespace Seele;
using namespace Seele::Editor;

SceneView::SceneView(Gfx::PGraphics graphics, PWindow owner, const ViewportCreateInfo& createInfo)
    : View(graphics, owner, createInfo, "SceneView"), scene(new Scene(graphics)), cameraSystem(createInfo.dimensions, Vector(0, 0, 10)) {
    cameraSystem.update(viewportCamera, static_cast<float>(Gfx::getCurrentFrameDelta()));
    renderGraph.addPass(new DepthCullingPass(graphics, scene));
    renderGraph.addPass(new LightCullingPass(graphics, scene));
    renderGraph.addPass(new BasePass(graphics, scene));
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();
}

SceneView::~SceneView() {}

void SceneView::beginUpdate() {
    // co_return;
}

void SceneView::update() {
    cameraSystem.update(viewportCamera, static_cast<float>(Gfx::getCurrentFrameDelta()));
    // co_return;
}

void SceneView::commitUpdate() {}

void SceneView::prepareRender() {}

void SceneView::render() { renderGraph.render(viewportCamera, Component::Transform()); }

void SceneView::keyCallback(KeyCode code, InputAction action, KeyModifier) { cameraSystem.keyCallback(code, action); }

void SceneView::mouseMoveCallback(double xPos, double yPos) { cameraSystem.mouseMoveCallback(xPos, yPos); }

void SceneView::mouseButtonCallback(MouseButton button, InputAction action, KeyModifier) {
    cameraSystem.mouseButtonCallback(button, action);
}

void SceneView::scrollCallback(double, double) {}

void SceneView::fileCallback(int, const char**) {}
