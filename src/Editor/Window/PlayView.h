#pragma once
#include "Window/GameView.h"

namespace Seele {
namespace Editor {
class PlayView : public GameView {
  public:
    PlayView(Gfx::PGraphics graphics, PWindow window, const ViewportCreateInfo& createInfo, std::filesystem::path dllPath);
    virtual ~PlayView();
    virtual void beginUpdate() override;
    virtual void update() override;
    virtual void commitUpdate() override;

    virtual void prepareRender() override;
    virtual void render() override;
    
    virtual void keyCallback(Seele::KeyCode code, Seele::InputAction action, Seele::KeyModifier modifier) override;

  private:
};
DECLARE_REF(PlayView)
} // namespace Editor
} // namespace Seele