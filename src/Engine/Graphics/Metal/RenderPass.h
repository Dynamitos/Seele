#pragma once
#include "Graphics/RenderTarget.h"
#include "Graphics.h"

namespace Seele 
{
namespace Metal 
{
class RenderPass : public Gfx::RenderPass
{
public:
    RenderPass(PGraphics graphics, Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport viewport);
    virtual ~RenderPass();
private:
    PGraphics graphics;
    Gfx::PViewport viewport;
};
DEFINE_REF(RenderPass)
} // namespace Metal
} // namespace Seele