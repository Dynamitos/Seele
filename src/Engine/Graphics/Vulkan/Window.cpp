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

void glfwKeyCallback(GLFWwindow* handle, int key, int, int action, int modifier)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->keyCallback((KeyCode)key, (InputAction)action, (KeyModifier)modifier);
}

void glfwMouseMoveCallback(GLFWwindow* handle, double xpos, double ypos)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->mouseMoveCallback(xpos, ypos);
}

void glfwMouseButtonCallback(GLFWwindow* handle, int button, int action, int modifier)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->mouseButtonCallback((MouseButton)button, (InputAction)action, (KeyModifier)modifier);
}

void glfwScrollCallback(GLFWwindow* handle, double xoffset, double yoffset)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->scrollCallback(xoffset, yoffset);
}

void glfwFileCallback(GLFWwindow* handle, int count, const char** paths)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->fileCallback(count, paths);
}

void glfwCloseCallback(GLFWwindow* handle)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->closeCallback();
}

Window::Window(PGraphics graphics, const WindowCreateInfo &createInfo)
    : graphics(graphics)
    , preferences(createInfo)
    , instance(graphics->getInstance())
    , swapchain(VK_NULL_HANDLE)
    , numSamples(createInfo.numSamples)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *handle = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, nullptr, nullptr);
    windowHandle = handle;
    glfwSetWindowUserPointer(handle, this);

    glfwSetKeyCallback(handle, &glfwKeyCallback);
    glfwSetCursorPosCallback(handle, &glfwMouseMoveCallback);
    glfwSetMouseButtonCallback(handle, &glfwMouseButtonCallback);
    glfwSetScrollCallback(handle, &glfwScrollCallback);
    glfwSetDropCallback(handle, &glfwFileCallback);
    glfwSetWindowCloseCallback(handle, &glfwCloseCallback);

    glfwCreateWindowSurface(instance, handle, nullptr, &surface);
    
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics->getPhysicalDevice(), surface, &capabilities);
    
    uint32 numFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(graphics->getPhysicalDevice(), surface, &numFormats, nullptr);
    supportedFormats.resize(numFormats);
    vkGetPhysicalDeviceSurfaceFormatsKHR(graphics->getPhysicalDevice(), surface, &numFormats, supportedFormats.data());

    uint32 numPresentModes;
    vkGetPhysicalDeviceSurfacePresentModesKHR(graphics->getPhysicalDevice(), surface, &numPresentModes, nullptr);
    supportedPresentModes.resize(numFormats);
    vkGetPhysicalDeviceSurfacePresentModesKHR(graphics->getPhysicalDevice(), surface, &numPresentModes, supportedPresentModes.data());
    chooseSwapSurfaceFormat();
    framebufferFormat = cast(format.format);
    chooseSwapPresentMode();
    chooseSwapExtent();
    framebufferWidth = extent.width;
    framebufferHeight = extent.height;
    sampleFlags = createInfo.numSamples;
    createSwapChain();
}

Window::~Window()
{
    vkDestroySwapchainKHR(graphics->getDevice(), swapchain, nullptr);
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(static_cast<GLFWwindow *>(windowHandle));
}

void Window::beginFrame()
{
    glfwPollEvents();
    vkAcquireNextImageKHR(graphics->getDevice(), swapchain, std::numeric_limits<uint64>::max(), imageAvailableSemaphores[currentSemaphoreIndex]->getHandle(), VK_NULL_HANDLE, &currentImageIndex);
    swapChainTextures[currentImageIndex]->changeLayout(Gfx::SE_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    graphics->getGraphicsCommands()->getCommands()->waitForSemaphore(VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, imageAvailableSemaphores[currentSemaphoreIndex]);
}

void Window::endFrame()
{
    swapChainTextures[currentImageIndex]->changeLayout(Gfx::SE_IMAGE_LAYOUT_PRESENT_SRC_KHR);
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
}

void Window::onWindowCloseEvent()
{
}

Gfx::PTexture2D Window::getBackBuffer()
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
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = capabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = presentMode,
        .clipped = VK_TRUE,
        .oldSwapchain = swapchain,
    };
    VK_CHECK(vkCreateSwapchainKHR(graphics->getDevice(), &createInfo, nullptr, &swapchain));

    uint32 numImages;
    vkGetSwapchainImagesKHR(graphics->getDevice(), swapchain, &numImages, swapChainImages.data());

    assert(numImages == Gfx::numFramesBuffered);
    for (uint32 i = 0; i < numImages; ++i)
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
                .usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
            }, swapChainImages[i]);
        imageAvailableSemaphores[i] = new Semaphore(graphics);
        renderingDoneSemaphores[i] = new Semaphore(graphics);
    }
}

Viewport::Viewport(PGraphics graphics, PWindow owner, const ViewportCreateInfo &viewportInfo)
    : Gfx::Viewport(owner, viewportInfo), graphics(graphics)
{
    handle.width = static_cast<float>(sizeX);
    handle.height = static_cast<float>(sizeY);
    handle.x = static_cast<float>(offsetX);
    handle.y = static_cast<float>(offsetY) + handle.height;
    handle.height = -handle.height;
    handle.minDepth =  0.f;
    handle.maxDepth =  1.f;
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