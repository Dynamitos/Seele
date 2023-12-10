#pragma once
#include "Resources.h"
#include "Texture.h"
#include "Window.h"

namespace Seele
{
namespace Gfx
{
class RenderTargetAttachment
{
public:
    RenderTargetAttachment(PTexture2D texture,
        SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
        SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
        SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE);
    ~RenderTargetAttachment();
    PTexture2D getTexture()
    {
        return texture;
    }
    SeFormat getFormat() const
    {
        return texture->getFormat();
    }
    SeSampleCountFlags getNumSamples() const
    {
        return texture->getNumSamples();
    }
    uint32 getWidth() const 
    { 
        return texture->getWidth(); 
    }
    uint32 getHeight() const 
    { 
        return texture->getHeight(); 
    }
    constexpr SeAttachmentLoadOp getLoadOp() const { return loadOp; }
    constexpr SeAttachmentStoreOp getStoreOp() const { return storeOp; }
    constexpr SeAttachmentLoadOp getStencilLoadOp() const { return stencilLoadOp; }
    constexpr SeAttachmentStoreOp getStencilStoreOp() const { return stencilStoreOp; }
    SeClearValue clear;
    SeColorComponentFlags componentFlags;
    SeAttachmentLoadOp loadOp;
    SeAttachmentStoreOp storeOp;
    SeAttachmentLoadOp stencilLoadOp;
    SeAttachmentStoreOp stencilStoreOp;
protected:
    PTexture2D texture;
};
DEFINE_REF(RenderTargetAttachment)

class RenderTargetLayout
{
public:
    Array<PRenderTargetAttachment> inputAttachments;
    Array<PRenderTargetAttachment> colorAttachments;
    Array<PRenderTargetAttachment> resolveAttachments;
    PRenderTargetAttachment depthAttachment;
    PRenderTargetAttachment depthResolveAttachment;
    uint32 width;
    uint32 height;
};
DEFINE_REF(RenderTargetLayout)

class RenderPass
{
public:
    RenderPass(ORenderTargetLayout layout) : layout(std::move(layout)) {}
    virtual ~RenderPass() {}
    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(RenderPass&&) = default;
    PRenderTargetLayout getLayout() const { return layout; }

protected:
    ORenderTargetLayout layout;
};
DEFINE_REF(RenderPass)
}
}