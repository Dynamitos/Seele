#pragma once
#include "Resources.h"
#include "Texture.h"

namespace Seele
{
namespace Gfx
{
class Window
{
public:
    Window(const WindowCreateInfo &createInfo);
    virtual ~Window();
    virtual void beginFrame() = 0;
    virtual void endFrame() = 0;
    virtual void onWindowCloseEvent() = 0;
    virtual PTexture2D getBackBuffer() = 0;
    virtual void setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback) = 0;
    virtual void setMouseMoveCallback(std::function<void(double, double)> callback) = 0;
    virtual void setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback) = 0;
    virtual void setScrollCallback(std::function<void(double, double)> callback) = 0;
    virtual void setFileCallback(std::function<void(int, const char**)> callback) = 0;
    virtual void setCloseCallback(std::function<void()> callback) = 0;
    constexpr SeFormat getSwapchainFormat() const
    {
        return windowState.pixelFormat;
    }
    constexpr SeSampleCountFlags getNumSamples() const
    {
        return windowState.numSamples;
    }
    constexpr uint32 getWidth() const
    {
        return windowState.width;
    }
    constexpr uint32 getHeight() const
    {
        return windowState.height;
    }

protected:
    WindowCreateInfo windowState;
};
DEFINE_REF(Window)

class Viewport
{
public:
    Viewport(PWindow owner, const ViewportCreateInfo &createInfo);
    virtual ~Viewport();
    virtual void resize(uint32 newX, uint32 newY) = 0;
    virtual void move(uint32 newOffsetX, uint32 newOffsetY) = 0;
    constexpr PWindow getOwner() const {return owner;}
    constexpr uint32 getWidth() const {return sizeX;}
    constexpr uint32 getHeight() const {return sizeY;}
    constexpr uint32 getOffsetX() const {return offsetX;}
    constexpr uint32 getOffsetY() const {return offsetY;}
    Matrix4 getProjectionMatrix() const;
protected:
    uint32 sizeX;
    uint32 sizeY;
    uint32 offsetX;
    uint32 offsetY;
    float fieldOfView;
    PWindow owner;
};
DEFINE_REF(Viewport)

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
        return owner->getWidth(); 
    }
    virtual uint32 getHeight() const 
    { 
        return owner->getHeight(); 
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