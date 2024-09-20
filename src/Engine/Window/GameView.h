#pragma once
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/RenderPass/BasePass.h"
#include "Graphics/Resources.h"
#include "Scene/Scene.h"
#include "System/KeyboardInput.h"
#include "Window/View.h"

#ifdef WIN32
#include "Platform/Windows/GameInterface.h" // TODO
#else
#include "Platform/Linux/GameInterface.h"
#endif

namespace Seele {
class GameView : public View {
  public:
    GameView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath);
    virtual ~GameView();
    virtual void beginUpdate() override;
    virtual void update() override;
    virtual void commitUpdate() override;

    virtual void prepareRender() override;
    virtual void render() override;

    void reloadGame();

  protected:
    virtual void applyArea(URect rect) override;
    OScene scene;
    GameInterface gameInterface;
    RenderGraph renderGraph;

    PSystemGraph systemGraph;
    System::PKeyboardInput keyboardSystem;
    struct ViewParameter {
        Matrix4 viewMatrix;
        Matrix4 inverseViewMatrix;
        Matrix4 projectionMatrix;
        Matrix4 inverseProjection;
        Vector4 cameraPosition;
        Vector2 screenDimensions;
    } viewParams;
    Gfx::ODescriptorLayout viewParamsLayout;
    Gfx::OUniformBuffer viewParamsBuffer;
    Gfx::PDescriptorSet viewParamsSet;
    Gfx::OPipelineLayout layout;
    Gfx::OTaskShader task;
    Gfx::OMeshShader mesh;
    Gfx::OVertexInput input;
    Gfx::OVertexShader vert;
    Gfx::OFragmentShader frag;
    Gfx::ORenderPass renderPass;
    Gfx::PGraphicsPipeline pipeline;

    OBasePass basePass;
    float updateTime = 0;

    virtual void keyCallback(Seele::KeyCode code, Seele::InputAction action, Seele::KeyModifier modifier) override;
    virtual void mouseMoveCallback(double xPos, double yPos) override;
    virtual void mouseButtonCallback(Seele::MouseButton button, Seele::InputAction action, Seele::KeyModifier modifier) override;
    virtual void scrollCallback(double xOffset, double yOffset) override;
    virtual void fileCallback(int count, const char** paths) override;
};
DEFINE_REF(GameView)
} // namespace Seele
