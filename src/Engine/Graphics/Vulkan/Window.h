#pragma once
#include "Graphics.h"
#include "Graphics/RenderTarget.h"
#include "Resources.h"
#include "Texture.h"


namespace Seele {
namespace Vulkan {
class Window : public Gfx::Window {
  public:
    Window(PGraphics graphics, const WindowCreateInfo& createInfo);
    virtual ~Window();
    virtual void pollInput() override;
    virtual void show() override;
    virtual void beginFrame() override;
    virtual void endFrame() override;
    virtual Gfx::PTexture2D getBackBuffer() const override;
    virtual void onWindowCloseEvent() override;
    virtual void setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback) override;
    virtual void setMouseMoveCallback(std::function<void(double, double)> callback) override;
    virtual void setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback) override;
    virtual void setScrollCallback(std::function<void(double, double)> callback) override;
    virtual void setFileCallback(std::function<void(int, const char**)> callback) override;
    virtual void setCloseCallback(std::function<void()> callback) override;
    virtual void setResizeCallback(std::function<void(uint32, uint32)> callback) override;

    void keyPress(KeyCode code, InputAction action, KeyModifier modifier);
    void mouseMove(double x, double y);
    void mouseButton(MouseButton button, InputAction action, KeyModifier modifier);
    void scroll(double x, double y);
    void fileDrop(int num, const char** files);
    void close();
    void resize(int width, int height);

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
    void* windowHandle;
    StaticArray<VkImage, Gfx::numFramesBuffered> swapChainImages;
    StaticArray<OTexture2D, Gfx::numFramesBuffered> swapChainTextures;
    StaticArray<OFence, Gfx::numFramesBuffered> imageAvailableFences;
    StaticArray<OSemaphore, Gfx::numFramesBuffered> imageAvailableSemaphores;
    StaticArray<OSemaphore, Gfx::numFramesBuffered> renderingDoneSemaphores;
    uint32 currentImageIndex = 0;
    uint32 currentSemaphoreIndex = 0;

    std::function<void(KeyCode, InputAction, KeyModifier)> keyCallback;
    std::function<void(double, double)> mouseMoveCallback;
    std::function<void(MouseButton, InputAction, KeyModifier)> mouseButtonCallback;
    std::function<void(double, double)> scrollCallback;
    std::function<void(int, const char**)> fileCallback;
    std::function<void()> closeCallback;
    std::function<void(uint32, uint32)> resizeCallback;
};
DEFINE_REF(Window)

class Viewport : public Gfx::Viewport {
  public:
    Viewport(PWindow owner, const ViewportCreateInfo& createInfo);
    virtual ~Viewport();
    virtual void resize(uint32 newX, uint32 newY);
    virtual void move(uint32 newOffsetX, uint32 newOffsetY);
    VkViewport getHandle() const { return handle; }

  private:
    VkViewport handle;
};
DECLARE_REF(Viewport)
} // namespace Vulkan
} // namespace Seele