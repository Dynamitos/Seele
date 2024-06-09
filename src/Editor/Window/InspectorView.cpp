#include "InspectorView.h"
#include "Actor/Actor.h"
#include "Asset/AssetRegistry.h"
#include "Asset/FontLoader.h"
#include "Graphics/Graphics.h"
#include "UI/System.h"
#include "Window/Window.h"


using namespace Seele;
using namespace Seele::Editor;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo)
    : View(graphics, std::move(window), std::move(createInfo), "InspectorView")
      //, renderGraph(RenderGraphBuilder::build(
      //    UIPass(graphics),
      //    TextPass(graphics)
      //))
      ,
      uiSystem(new UI::System()) {
    // renderGraph.updateViewport(viewport);
    uiSystem->updateViewport(viewport);
}

InspectorView::~InspectorView() {}

void InspectorView::beginUpdate() {
    // co_return;
}

void InspectorView::update() {
    // co_return;
}

void InspectorView::commitUpdate() {}

void InspectorView::prepareRender() {}

void InspectorView::render() {}

void InspectorView::keyCallback(KeyCode, InputAction, KeyModifier) {}

void InspectorView::mouseMoveCallback(double, double) {}

void InspectorView::mouseButtonCallback(MouseButton, InputAction, KeyModifier) {}

void InspectorView::scrollCallback(double, double) {}

void InspectorView::fileCallback(int, const char**) {}
