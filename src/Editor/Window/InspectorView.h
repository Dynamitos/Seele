#pragma once
#include "Graphics/RenderPass/RenderGraph.h"
#include "Graphics/RenderPass/UIPass.h"
#include "Window/View.h"
#include "UI/System.h"

namespace Seele {
DECLARE_REF(Actor)
namespace Editor {
class InspectorView : public View {
  public:
    InspectorView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo);
    virtual ~InspectorView();

    virtual void beginUpdate() override;
    virtual void update() override;
    virtual void commitUpdate() override;

    virtual void prepareRender() override;
    virtual void render() override;


  protected:
    virtual void applyArea(URect area) override;
    UI::PSystem uiSystem;
    RenderGraph renderGraph;
    Component::Camera cam;

    virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) override;
    virtual void mouseMoveCallback(double xPos, double yPos) override;
    virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) override;
    virtual void scrollCallback(double xOffset, double yOffset) override;
    virtual void fileCallback(int count, const char** paths) override;
};
DEFINE_REF(InspectorView)
} // namespace Editor
} // namespace Seele
