#pragma once
#include "Graphics/RenderTarget.h"
#include "Graphics.h"
#include "Texture.h"
#include "Resources.h"

namespace Seele
{
namespace Vulkan
{
class Window : public Gfx::Window
{
public:
    Window(PGraphics graphics, const WindowCreateInfo &createInfo);
    virtual ~Window();
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual Gfx::PTexture2D getBackBuffer() override;
    virtual void onWindowCloseEvent() override;
    virtual void setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback) override;
    virtual void setMouseMoveCallback(std::function<void(double, double)> callback) override;
    virtual void setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback) override;
    virtual void setScrollCallback(std::function<void(double, double)> callback) override;
    virtual void setFileCallback(std::function<void(int, const char**)> callback) override;
    virtual void setCloseCallback(std::function<void()> callback);

    VkFormat getPixelFormat() const
    {
        return cast(windowState.pixelFormat);
    }

    std::function<void(KeyCode, InputAction, KeyModifier)> keyCallback;
    std::function<void(double, double)> mouseMoveCallback;
    std::function<void(MouseButton, InputAction, KeyModifier)> mouseButtonCallback;
    std::function<void(double, double)> scrollCallback;
    std::function<void(int, const char**)> fileCallback;
    std::function<void()> closeCallback;
protected:
    void advanceBackBuffer();
    void recreateSwapchain(const WindowCreateInfo &createInfo);
    void present();
    void destroySwapchain();
    void createSwapchain();
    void chooseSurfaceFormat(const Array<VkSurfaceFormatKHR> &available, Gfx::SeFormat preferred);
    void choosePresentMode(const Array<VkPresentModeKHR> &modes);

    OTexture2D backBufferImages[Gfx::numFramesBuffered];
    OSemaphore renderFinished[Gfx::numFramesBuffered];
    OSemaphore imageAcquired[Gfx::numFramesBuffered];
    PSemaphore imageAcquiredSemaphore;

    PGraphics graphics;
    VkInstance instance;
    VkSwapchainKHR swapchain;
    VkSampleCountFlags numSamples;
    VkPresentModeKHR presentMode;
    VkSurfaceKHR surface;
    VkSurfaceFormatKHR surfaceFormat;
    void *windowHandle;
    int32 currentImageIndex;
    int32 acquiredImageIndex;
    int32 preAcquiredImageIndex;
    int32 semaphoreIndex;
};
DEFINE_REF(Window)

class Viewport : public Gfx::Viewport
{
public:
    Viewport(PGraphics graphics, PWindow owner, const ViewportCreateInfo &createInfo);
    virtual ~Viewport();
    virtual void resize(uint32 newX, uint32 newY);
    virtual void move(uint32 newOffsetX, uint32 newOffsetY);
    VkViewport getHandle() const { return handle; }
private:
    VkViewport handle;
    PGraphics graphics;
    friend class Graphics;
};
DECLARE_REF(Viewport)
}
}