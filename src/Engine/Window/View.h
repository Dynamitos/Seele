#pragma once
#include "Graphics/RenderPass/RenderGraph.h"

namespace Seele {
DECLARE_REF(Window)
// A view is a part of the window, which can be anything from a scene viewport to an inspector
class View {
  public:
    View(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string name);
    virtual ~View();

    virtual void beginUpdate() = 0;
    virtual void update() = 0;
    virtual void commitUpdate() = 0;

    virtual void prepareRender() = 0;
    virtual void render() = 0;
    void resize(URect area);
    void setFocused();

    const std::string& getName();

  protected:
    virtual void applyArea(URect area) = 0;
    Gfx::PGraphics graphics;
    Gfx::OViewport viewport;
    ViewportCreateInfo createInfo;
    PWindow owner;
    std::string name;

    virtual void keyCallback(KeyCode code, InputAction action, KeyModifier modifier) = 0;
    virtual void mouseMoveCallback(double xPos, double yPos) = 0;
    virtual void mouseButtonCallback(MouseButton button, InputAction action, KeyModifier modifier) = 0;
    virtual void scrollCallback(double xOffset, double yOffset) = 0;
    virtual void fileCallback(int count, const char** paths) = 0;
    friend class Window;
};

DEFINE_REF(View)
} // namespace Seele