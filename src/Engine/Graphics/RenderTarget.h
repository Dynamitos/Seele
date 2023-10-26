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
    virtual PTexture2D getBackBuffer() const = 0;
    virtual void setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback) = 0;
    virtual void setMouseMoveCallback(std::function<void(double, double)> callback) = 0;
    virtual void setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback) = 0;
    virtual void setScrollCallback(std::function<void(double, double)> callback) = 0;
    virtual void setFileCallback(std::function<void(int, const char**)> callback) = 0;
    virtual void setCloseCallback(std::function<void()> callback) = 0;
    SeFormat getSwapchainFormat() const
    {
        return windowState.pixelFormat;
    }
    SeSampleCountFlags getNumSamples() const
    {
        return windowState.numSamples;
    }
    uint32 getSizeX() const
    {
        return windowState.width;
    }
    uint32 getSizeY() const
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
    constexpr uint32 getSizeX() const {return sizeX;}
    constexpr uint32 getSizeY() const {return sizeY;}
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
                           SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
        : loadOp(loadOp)
        , storeOp(storeOp)
        , stencilLoadOp(stencilLoadOp)
        , stencilStoreOp(stencilStoreOp)
        , texture(texture)
        , clear()
        , componentFlags(0)
    {
    }
    virtual ~RenderTargetAttachment()
    {
    }
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
    virtual uint32 getSizeX() const 
    { 
        return texture->getSizeX(); 
    }
    virtual uint32 getSizeY() const 
    { 
        return texture->getSizeY(); 
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
                        SeAttachmentStoreOp stencilStoreOp = SE_ATTACHMENT_STORE_OP_DONT_CARE)
        : RenderTargetAttachment(nullptr, loadOp, storeOp, stencilLoadOp, stencilStoreOp), owner(owner)
    {
        clear.color.float32[0] = 0.0f;
        clear.color.float32[1] = 0.0f;
        clear.color.float32[2] = 0.0f;
        clear.color.float32[3] = 1.0f;
        componentFlags = SE_COLOR_COMPONENT_R_BIT | SE_COLOR_COMPONENT_G_BIT | SE_COLOR_COMPONENT_B_BIT | SE_COLOR_COMPONENT_A_BIT;
    }
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
    virtual uint32 getSizeX() const 
    { 
        return owner->getSizeX(); 
    }
    virtual uint32 getSizeY() const 
    { 
        return owner->getSizeY(); 
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
    RenderPass(PRenderTargetLayout layout) : layout(layout) {}
    virtual ~RenderPass() {}
    inline PRenderTargetLayout getLayout() const { return layout; }

protected:
    PRenderTargetLayout layout;
};
DEFINE_REF(RenderPass)
}
}