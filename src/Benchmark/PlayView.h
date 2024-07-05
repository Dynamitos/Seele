#pragma once
#include "Window/GameView.h"

namespace Seele {
class PlayView : public GameView {
  public:
    PlayView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::string dllPath, bool useMeshCulling);
    virtual ~PlayView();
    virtual void beginUpdate() override;
    virtual void update() override;
    virtual void commitUpdate() override;

    virtual void prepareRender() override;
    virtual void render() override;
    
    virtual void keyCallback(Seele::KeyCode code, Seele::InputAction action, Seele::KeyModifier modifier) override;

  private:
    std::thread queryThread;
};
DECLARE_REF(PlayView)
} // namespace Seele