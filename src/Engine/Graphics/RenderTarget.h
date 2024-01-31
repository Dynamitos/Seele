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
        SeImageLayout initialLayout,
        SeImageLayout finalLayout,
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
    constexpr SeAttachmentLoadOp getLoadOp() const { return loadOp; }
    constexpr SeAttachmentStoreOp getStoreOp() const { return storeOp; }
    constexpr SeAttachmentLoadOp getStencilLoadOp() const { return stencilLoadOp; }
    constexpr SeAttachmentStoreOp getStencilStoreOp() const { return stencilStoreOp; }
    constexpr SeImageLayout getInitialLayout() const { return initialLayout; }
    constexpr SeImageLayout getFinalLayout() const { return finalLayout; }
    constexpr void setLoadOp(SeAttachmentLoadOp val) { loadOp = val; }
    constexpr void setStoreOp(SeAttachmentStoreOp val) { storeOp = val; }
    constexpr void setStencilLoadOp(SeAttachmentLoadOp val) { stencilLoadOp = val; }
    constexpr void setStencilStoreOp(SeAttachmentStoreOp val) { stencilStoreOp = val; }
    constexpr void setInitialLayout(SeImageLayout val) { initialLayout = val; }
    constexpr void setFinalLayout(SeImageLayout val) { finalLayout = val; }
    SeClearValue clear;
    SeColorComponentFlags componentFlags;
protected:
    PTexture2D texture;
    SeImageLayout initialLayout;
    SeImageLayout finalLayout;
    SeAttachmentLoadOp loadOp;
    SeAttachmentStoreOp storeOp;
    SeAttachmentLoadOp stencilLoadOp;
    SeAttachmentStoreOp stencilStoreOp;
};
DEFINE_REF(RenderTargetAttachment)

class SwapchainAttachment : public RenderTargetAttachment
{
public:
    SwapchainAttachment(PViewport viewport,
        SeImageLayout initialLayout,
        SeImageLayout finalLayout,
        SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
        SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
        SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
        : RenderTargetAttachment(nullptr, initialLayout, finalLayout, loadOp, storeOp, stencilLoadOp, stencilStoreOp)
        , viewport(viewport)
    {}
    virtual ~SwapchainAttachment()
    {
    }
    virtual PTexture2D getTexture()
    {
        return viewport->getOwner()->getBackBuffer();
    }
    virtual SeFormat getFormat() const
    {
        return viewport->getOwner()->getSwapchainFormat();
    }
    virtual SeSampleCountFlags getNumSamples() const
    {
        return viewport->getSamples();
    }
    virtual uint32 getWidth() const 
    { 
        return viewport->getWidth(); 
    }
    virtual uint32 getHeight() const 
    { 
        return viewport->getHeight(); 
    }
private:
    PViewport viewport;
};

struct RenderTargetLayout
{
    Array<PRenderTargetAttachment> inputAttachments;
    Array<PRenderTargetAttachment> colorAttachments;
    Array<PRenderTargetAttachment> resolveAttachments;
    PRenderTargetAttachment depthAttachment;
    PRenderTargetAttachment depthResolveAttachment;
};

struct SubPassDependency
{
    uint32 srcSubpass;
    uint32 dstSubpass;
    SePipelineStageFlags srcStage;
    SePipelineStageFlags dstStage;
    SeAccessFlags srcAccess;
    SeAccessFlags dstAccess;
};

class RenderPass
{
public:
    RenderPass(RenderTargetLayout layout, Array<SubPassDependency> dependencies) 
        : layout(std::move(layout))
        , dependencies(std::move(dependencies))
    {}
    virtual ~RenderPass() {}
    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(RenderPass&&) = default;
    const RenderTargetLayout& getLayout() const { return layout; }

protected:
    RenderTargetLayout layout;
    Array<SubPassDependency> dependencies;
};
DEFINE_REF(RenderPass)
}
}