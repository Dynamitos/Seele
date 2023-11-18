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
    virtual ~RenderTargetAttachment();
    virtual PTexture2D getTexture()
    {
        return texture;
    }
    virtual SeFormat getFormat() const
    {
        return texture->getFormat();
    }
    virtual SeSampleCountFlags getNumSamples() const
    {
        return texture->getNumSamples();
    }
    virtual uint32 getWidth() const 
    { 
        return texture->getWidth(); 
    }
    virtual uint32 getHeight() const 
    { 
        return texture->getHeight(); 
    }
    inline SeAttachmentLoadOp getLoadOp() const { return loadOp; }
    inline SeAttachmentStoreOp getStoreOp() const { return storeOp; }
    inline SeAttachmentLoadOp getStencilLoadOp() const { return stencilLoadOp; }
    inline SeAttachmentStoreOp getStencilStoreOp() const { return stencilStoreOp; }
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

class SwapchainAttachment : public RenderTargetAttachment
{
public:
    SwapchainAttachment(PWindow owner,
        SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
        SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
        SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE);
    virtual ~SwapchainAttachment();
    virtual PTexture2D getTexture() override
    {
        return owner->getBackBuffer();
    }
    virtual SeFormat getFormat() const override
    {
        return owner->getSwapchainFormat();
    }
    virtual SeSampleCountFlags getNumSamples() const override
    {
        return owner->getNumSamples();
    }
    virtual uint32 getWidth() const 
    { 
        return owner->getFramebufferWidth(); 
    }
    virtual uint32 getHeight() const 
    { 
        return owner->getFramebufferHeight(); 
    }
private:
    PWindow owner;
};
DEFINE_REF(SwapchainAttachment)

class RenderTargetLayout
{
public:
    RenderTargetLayout();
    RenderTargetLayout(PRenderTargetAttachment depthAttachment);
    RenderTargetLayout(PRenderTargetAttachment colorAttachment, PRenderTargetAttachment depthAttachment);
    RenderTargetLayout(Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachmet);
    RenderTargetLayout(Array<PRenderTargetAttachment> inputAttachments, Array<PRenderTargetAttachment> colorAttachments, PRenderTargetAttachment depthAttachment);
    Array<PRenderTargetAttachment> inputAttachments;
    Array<PRenderTargetAttachment> colorAttachments;
    Array<PRenderTargetAttachment> resolveAttachments;
    PRenderTargetAttachment depthAttachment;
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