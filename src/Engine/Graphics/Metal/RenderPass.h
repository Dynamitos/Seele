#pragma once
#include "Graphics/RenderTarget.h"
#include "Resources.h"

namespace Seele 
{
namespace Metal 
{
DECLARE_REF(Graphics)
class RenderPass : public Gfx::RenderPass
{
public:
    RenderPass(PGraphics graphics, Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport viewport);
    virtual ~RenderPass();
    MTL::RenderPassDescriptor* getDescriptor() const 
    {
        return renderPass;
    }
private:
    PGraphics graphics;
    Gfx::PViewport viewport;
    MTL::RenderPassDescriptor* renderPass;
    MTL::RenderPassDepthAttachmentDescriptor* depth;
    MTL::RenderPassStencilAttachmentDescriptor* stencil;
};
DEFINE_REF(RenderPass)
} // namespace Metal
} // namespace Seele