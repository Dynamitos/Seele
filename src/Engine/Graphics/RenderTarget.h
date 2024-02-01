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
    RenderTargetAttachment()
    {}
    RenderTargetAttachment(PTexture2D texture,
        SeImageLayout initialLayout,
        SeImageLayout finalLayout,
        SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
        SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
        SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE);

    RenderTargetAttachment(PViewport viewport,
        SeImageLayout initialLayout,
        SeImageLayout finalLayout,
        SeAttachmentLoadOp loadOp = SE_ATTACHMENT_LOAD_OP_LOAD,
        SeAttachmentStoreOp storeOp = SE_ATTACHMENT_STORE_OP_STORE,
        SeAttachmentLoadOp stencilLoadOp = SE_ATTACHMENT_LOAD_OP_DONT_CARE,
        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE);
    ~RenderTargetAttachment();
    PTexture2D getTexture()
    {
        if(viewport != nullptr)
        {
            return viewport->getOwner()->getBackBuffer();
        }
        return texture;
    }
    SeFormat getFormat() const
    {
        if(viewport != nullptr)
        {
            return viewport->getOwner()->getSwapchainFormat();
        }
        return texture->getFormat();
    }
    SeSampleCountFlags getNumSamples() const
    {
        if(viewport != nullptr)
        {
            return viewport->getSamples();
        }
        return texture->getNumSamples();
    }
    uint32 getWidth() const 
    { 
        if(viewport != nullptr)
        {
            return viewport->getWidth();
        }
        return texture->getWidth(); 
    }
    uint32 getHeight() const 
    { 
        if(viewport != nullptr)
        {
            return viewport->getHeight();
        }
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
    PTexture2D texture = nullptr;
    PViewport viewport = nullptr;
    SeImageLayout initialLayout;
    SeImageLayout finalLayout;
    SeAttachmentLoadOp loadOp;
    SeAttachmentStoreOp storeOp;
    SeAttachmentLoadOp stencilLoadOp;
    SeAttachmentStoreOp stencilStoreOp;
};

struct RenderTargetLayout
{
    Array<RenderTargetAttachment> inputAttachments;
    Array<RenderTargetAttachment> colorAttachments;
    Array<RenderTargetAttachment> resolveAttachments;
    RenderTargetAttachment depthAttachment;
    RenderTargetAttachment depthResolveAttachment;
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