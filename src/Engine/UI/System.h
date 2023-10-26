#pragma once
#include "MinimalEngine.h"
#include "RenderHierarchy.h"
#include "Graphics/RenderPass/UIPass.h"
#include "Graphics/RenderPass/TextPass.h"

namespace Seele
{
namespace UI
{
DECLARE_REF(Panel)
class System
{
public:
    System();
    virtual ~System();
    void update();
    void updateViewport(Gfx::PViewport viewport);
    Component::Camera getVirtualCamera() const;
private:
    Component::Camera virtualCamera;
    PPanel rootPanel;
    RenderHierarchy hierarchy;
};
DEFINE_REF(System)
} // namespace UI
} // namespace Seele
