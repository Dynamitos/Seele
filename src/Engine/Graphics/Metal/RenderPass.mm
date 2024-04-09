#include "RenderPass.h"
#include "Graphics/RenderTarget.h"
#include "Metal/MTLRenderPass.hpp"

using namespace Seele;
using namespace Seele::Metal;

RenderPass::RenderPass(PGraphics graphics, Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport viewport)
    : Gfx::RenderPass(layout, dependencies)
    , graphics(graphics)
    , viewport(viewport)
{
    
}
RenderPass::~RenderPass()
{
}