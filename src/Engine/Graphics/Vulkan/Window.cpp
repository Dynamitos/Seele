#include "Window.h"
#include "Resources.h"
#include "Graphics.h"
#include "Enums.h"
#include "Command.h"
#include <GLFW/glfw3.h>

using namespace Seele;
using namespace Seele::Vulkan;

double currentFrameDelta = 0;
double Gfx::getCurrentFrameDelta()
{
    return currentFrameDelta;
}

uint32 currentFrameIndex = 0;
uint32 Gfx::getCurrentFrameIndex()
{
    return currentFrameIndex;
}

void glfwKeyCallback(GLFWwindow* handle, int key, int, int action, int modifier)
{
    if (key == -1)
    {
        return;
    }
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->keyPress((KeyCode)key, (InputAction)action, (KeyModifier)modifier);
}

void glfwMouseMoveCallback(GLFWwindow* handle, double xpos, double ypos)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->mouseMove(xpos, ypos);
}

void glfwMouseButtonCallback(GLFWwindow* handle, int button, int action, int modifier)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->mouseButton((MouseButton)button, (InputAction)action, (KeyModifier)modifier);
}

void glfwScrollCallback(GLFWwindow* handle, double xoffset, double yoffset)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->scroll(xoffset, yoffset);
}

void glfwFileCallback(GLFWwindow* handle, int count, const char** paths)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->fileDrop(count, paths);
}

void glfwCloseCallback(GLFWwindow* handle)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->close();
}

void glfwFramebufferResizeCallback(GLFWwindow* handle, int width, int height)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->resize(width, height);
}

Window::Window(PGraphics graphics, const WindowCreateInfo &createInfo)
    : graphics(graphics)
    , preferences(createInfo)
    , instance(graphics->getInstance())
    , swapchain(VK_NULL_HANDLE)
{
    float xscale, yscale;
    glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &xscale, &yscale);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *handle = glfwCreateWindow(createInfo.width / xscale, createInfo.height / yscale, createInfo.title, nullptr, nullptr);
    windowHandle = handle;
    glfwSetWindowUserPointer(handle, this);

    glfwSetKeyCallback(handle, &glfwKeyCallback);
    glfwSetCursorPosCallback(handle, &glfwMouseMoveCallback);
    glfwSetMouseButtonCallback(handle, &glfwMouseButtonCallback);
    glfwSetScrollCallback(handle, &glfwScrollCallback);
    glfwSetDropCallback(handle, &glfwFileCallback);
    glfwSetWindowCloseCallback(handle, &glfwCloseCallback);
    glfwSetFramebufferSizeCallback(handle, &glfwFramebufferResizeCallback);
    //glfwSetWindowSizeCallback(handle, &glfwResizeCallback);

    glfwCreateWindowSurface(instance, handle, nullptr, &surface);
    
    querySurface();
    chooseSwapSurfaceFormat();
    framebufferFormat = cast(format.format);
    chooseSwapPresentMode();
    chooseSwapExtent();
    framebufferWidth = extent.width;
    framebufferHeight = extent.height;
    createSwapChain();
}

Window::~Window()
{
    vkDestroySwapchainKHR(graphics->getDevice(), swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(static_cast<GLFWwindow *>(windowHandle));
}

void Window::pollInput()
{
    glfwPollEvents();
}

void Window::beginFrame()
{
    imageAvailableFences[currentSemaphoreIndex]->wait(100000000);
    imageAvailableFences[currentSemaphoreIndex]->reset();
    vkAcquireNextImageKHR(graphics->getDevice(), swapchain, std::numeric_limits<uint64>::max(), imageAvailableSemaphores[currentSemaphoreIndex]->getHandle(), imageAvailableFences[currentSemaphoreIndex]->getHandle(), &currentImageIndex);
    graphics->getGraphicsCommands()->getCommands()->waitForSemaphore(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, imageAvailableSemaphores[currentSemaphoreIndex]);
}

void Window::endFrame()
{
    swapChainTextures[currentImageIndex]->changeLayout(Gfx::SE_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        Gfx::SE_ACCESS_COLOR_ATTACHMENT_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        Gfx::SE_ACCESS_MEMORY_READ_BIT, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    graphics->getGraphicsCommands()->submitCommands(renderingDoneSemaphores[currentSemaphoreIndex]);
    VkSemaphore renderDoneHandle = renderingDoneSemaphores[currentSemaphoreIndex]->getHandle();
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .pNext = nullptr,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &renderDoneHandle,
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &currentImageIndex,
        .pResults = nullptr,
    };
    VkResult r = vkQueuePresentKHR(graphics->getGraphicsCommands()->getQueue()->getHandle(), &presentInfo);
    if(r == VK_SUCCESS)
    { }
    else if (r == VK_ERROR_OUT_OF_DATE_KHR || r == VK_SUBOPTIMAL_KHR)
    {
        createSwapChain();
    }
    else
    {
        VK_CHECK(r);
    }
    currentSemaphoreIndex = (currentSemaphoreIndex + 1) % Gfx::numFramesBuffered;
    currentFrameIndex = currentSemaphoreIndex;
    //graphics->waitDeviceIdle();
}

void Window::onWindowCloseEvent()
{
}

Gfx::PTexture2D Window::getBackBuffer() const
{
    return PTexture2D(swapChainTextures[currentImageIndex]);
}

void Window::setKeyCallback(std::function<void(KeyCode, InputAction, KeyModifier)> callback)
{
    keyCallback = callback;
}

void Window::setMouseMoveCallback(std::function<void(double, double)> callback)
{
    mouseMoveCallback = callback;
}

void Window::setMouseButtonCallback(std::function<void(MouseButton, InputAction, KeyModifier)> callback)
{
    mouseButtonCallback = callback;
}

void Window::setScrollCallback(std::function<void(double, double)> callback) 
{
    scrollCallback = callback;
}

void Window::setFileCallback(std::function<void(int, const char**)> callback)
{
    fileCallback = callback;
}

void Window::setCloseCallback(std::function<void()> callback)
{
    closeCallback = callback;
}

void Window::setResizeCallback(std::function<void(uint32, uint32)> callback)
{
    resizeCallback = callback;
}

void Window::keyPress(KeyCode code, InputAction action, KeyModifier modifier)
{
    keyCallback(code, action, modifier);
}

void Window::mouseMove(double x, double y)
{
    mouseMoveCallback(x, y);
}

void Window::mouseButton(MouseButton button, InputAction action, KeyModifier modifier)
{
    mouseButtonCallback(button, action, modifier);
}

void Window::scroll(double x, double y)
{
    scrollCallback(x, y);
}

void Window::fileDrop(int num, const char** files)
{
    fileCallback(num, files);
}

void Window::close()
{
    closeCallback();
}

void Window::resize(int width, int height)
{
    if (width == 0 || height == 0)
    {
        paused = true;
        return;
    }
    paused = false;
    querySurface();
    chooseSwapSurfaceFormat();
    framebufferFormat = cast(format.format);
    chooseSwapPresentMode();
    chooseSwapExtent();
    framebufferWidth = extent.width;
    framebufferHeight = extent.height;
    createSwapChain();
    resizeCallback(width, height);
}

void Window::querySurface()
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics->getPhysicalDevice(), surface, &capabilities);

    uint32 numFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(graphics->getPhysicalDevice(), surface, &numFormats, nullptr);
    supportedFormats.resize(numFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(graphics->getPhysicalDevice(), surface, &numFormats, supportedFormats.data());

    uint32 numPresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(graphics->getPhysicalDevice(), surface, &numPresentModes, nullptr);
    supportedPresentModes.resize(numFormats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(graphics->getPhysicalDevice(), surface, &numPresentModes, supportedPresentModes.data());
}

void Window::chooseSwapSurfaceFormat()
{
    for (const auto& supportedFormat : supportedFormats)
    {
        if (supportedFormat.format == cast(preferences.preferredFormat))
        {
            format = supportedFormat;
            return;
        }
    }
    for (const auto& supportedFormat : supportedFormats)
    {
        if (supportedFormat.format == VK_FORMAT_R8G8B8A8_SRGB && supportedFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            format = supportedFormat;
            return;
        }
    }
    format = supportedFormats[0];
}

void Window::chooseSwapPresentMode()
{
    for (const auto& supportedPresentMode : supportedPresentModes)
    {
        if (supportedPresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = supportedPresentMode;
            return;
        }
    }
    presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void Window::chooseSwapExtent()
{
    if (capabilities.currentExtent.width != std::numeric_limits<uint32>::max()) {
        extent = capabilities.currentExtent;
        return;
    }
    int width, height;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(windowHandle), &width, &height);

    extent = {
        static_cast<uint32>(width),
        static_cast<uint32>(height),
    };
    extent.width = std::clamp(extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    extent.height = std::clamp(extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
}

void Window::createSwapChain()
{
    uint32 imageCount = Gfx::numFramesBuffered;
    VkSwapchainCreateInfoKHR createInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .pNext = nullptr,
        .surface = surface,
        .minImageCount = Gfx::numFramesBuffered,
        .imageFormat = format.format,
        .imageColorSpace = format.colorSpace,
        .imageExtent = extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = swapchain,
    };
    VK_CHECK(vkCreateSwapchainKHR(graphics->getDevice(), &createInfo, nullptr, &swapchain));

    VK_CHECK(vkGetSwapchainImagesKHR(graphics->getDevice(), swapchain, &imageCount, swapChainImages.data()));

    for (uint32 i = 0; i < imageCount; ++i)
    {
        swapChainTextures[i] = new Texture2D(graphics, TextureCreateInfo{
                .format = cast(format.format),
                .width = extent.width,
                .height = extent.height,
                .depth = 1,
                .mipLevels = 1,
                .layers = 1,
                .elements = 1,
                .samples = 1,
                .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | Gfx::SE_IMAGE_USAGE_TRANSFER_DST_BIT,
            }, swapChainImages[i]);
        if(imageAvailableFences[i] != nullptr)
        {
            imageAvailableFences[i]->wait(100000);
            imageAvailableFences[i]->reset();
        }
        imageAvailableFences[i] = new Fence(graphics);
        imageAvailableSemaphores[i] = new Semaphore(graphics);
        renderingDoneSemaphores[i] = new Semaphore(graphics);
    }
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo &viewportInfo)
    : Gfx::Viewport(owner, viewportInfo)
{
    handle.width = static_cast<float>(sizeX);
    handle.height = static_cast<float>(sizeY);
    handle.x = static_cast<float>(offsetX);
    handle.y = static_cast<float>(offsetY) + handle.height;
    handle.height = -handle.height;
    handle.minDepth =  1.f;
    handle.maxDepth =  0.f;
}

Viewport::~Viewport()
{
}

void Viewport::resize(uint32 newX, uint32 newY)
{
    sizeX = newX;
    sizeY = newY;
    handle.width = static_cast<float>(sizeX);
    handle.y = static_cast<float>(sizeY + offsetX);
    handle.height = -static_cast<float>(sizeY);
}

void Viewport::move(uint32 newOffsetX, uint32 newOffsetY)
{
    offsetX = newOffsetX;
    offsetY = newOffsetY;
    handle.x = static_cast<float>(offsetX);
    handle.y = static_cast<float>(offsetY + sizeY);
}
