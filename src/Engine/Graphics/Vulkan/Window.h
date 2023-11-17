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
    void resize(int width, int height);

    std::function<void(KeyCode, InputAction, KeyModifier)> keyCallback;
    std::function<void(double, double)> mouseMoveCallback;
    std::function<void(MouseButton, InputAction, KeyModifier)> mouseButtonCallback;
    std::function<void(double, double)> scrollCallback;
    std::function<void(int, const char**)> fileCallback;
    std::function<void()> closeCallback;
protected:
    void querySurface();
    void chooseSwapSurfaceFormat();
    void chooseSwapPresentMode();
    void chooseSwapExtent();
    void createSwapChain();
    PGraphics graphics;
    WindowCreateInfo preferences;
    VkInstance instance;
    VkSwapchainKHR swapchain;
    VkSampleCountFlags numSamples;
    VkSurfaceKHR surface;
    VkSurfaceCapabilitiesKHR capabilities;
    Array<VkSurfaceFormatKHR> supportedFormats;
    Array<VkPresentModeKHR> supportedPresentModes;
    // the values used for the current swapchain
    VkSurfaceFormatKHR format;
    VkPresentModeKHR presentMode;
    VkExtent2D extent;
    void *windowHandle;
    StaticArray<VkImage, Gfx::numFramesBuffered> swapChainImages;
    StaticArray<OTexture2D, Gfx::numFramesBuffered> swapChainTextures;
    StaticArray<OSemaphore, Gfx::numFramesBuffered> imageAvailableSemaphores;
    StaticArray<OSemaphore, Gfx::numFramesBuffered> renderingDoneSemaphores;
    uint32 currentImageIndex = 0;
    uint32 currentSemaphoreIndex = 0;
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