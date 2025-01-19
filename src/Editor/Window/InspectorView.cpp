#include "InspectorView.h"
#include "Actor/Actor.h"
#include "Asset/AssetRegistry.h"
#include "Asset/FontLoader.h"
#include "Graphics/Graphics.h"
#include "UI/Element/Div.h"
#include "UI/Element/Text.h"

using namespace Seele;
using namespace Seele::Editor;

InspectorView::InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo)
    : View(graphics, window, createInfo, "InspectorView"),
      uiSystem(new UI::System(
          new UI::Div<M_8>(UI::Attributes{}, {
                                                 new UI::Div<BG_Red, Inline>({},
                                                                             {
                                                                                 new UI::Text<Font_Arial, Text_9XL>("OtherTestT"),
                                                                             }),
                                                 new UI::Text<Font_Arial>("Test"),
                                             }))) {
    renderGraph.addPass(new UIPass(graphics, uiSystem));
    renderGraph.setViewport(viewport);
    renderGraph.createRenderPass();
}

InspectorView::~InspectorView() {}

void InspectorView::beginUpdate() {}

void InspectorView::update() {}

void InspectorView::commitUpdate() {}

void InspectorView::prepareRender() {}

void InspectorView::render() { renderGraph.render(Component::Camera()); }

void InspectorView::applyArea(URect area) {}

void InspectorView::keyCallback(KeyCode, InputAction, KeyModifier) {}

void InspectorView::mouseMoveCallback(double, double) {}

void InspectorView::mouseButtonCallback(MouseButton, InputAction, KeyModifier) {}

void InspectorView::scrollCallback(double, double) {}

void InspectorView::fileCallback(int, const char**) {}
