#include "VulkanGraphicsResources.h"
#include "VulkanGraphics.h"
#include "VulkanInitializer.h"
#include "VulkanGraphicsEnums.h"
#include "VulkanCommandBuffer.h"
#include <glfw/glfw3.h>

using namespace Seele;
using namespace Seele::Vulkan;

void glfwKeyCallback(GLFWwindow* handle, int key, int scancode, int action, int modifier)
{
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    std::cout << "glfw callback: " << key << std::endl;
    window->keyCallback((KeyCode)key, (KeyAction)action, (KeyModifier)modifier);
}

Window::Window(PGraphics graphics, const WindowCreateInfo &createInfo)
    : Gfx::Window(createInfo), graphics(graphics), instance(graphics->getInstance()), swapchain(VK_NULL_HANDLE), numSamples(createInfo.numSamples), pixelFormat(cast(createInfo.pixelFormat))
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    GLFWwindow *handle = glfwCreateWindow(createInfo.width, createInfo.height, createInfo.title, createInfo.bFullscreen ? glfwGetPrimaryMonitor() : nullptr, nullptr);
    windowHandle = handle;
    glfwSetWindowUserPointer(handle, this);

    glfwSetKeyCallback(handle, &glfwKeyCallback);

    glfwCreateWindowSurface(instance, handle, nullptr, &surface);

    uint32_t numQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(graphics->getPhysicalDevice(), &numQueueFamilies, nullptr);
    Array<VkQueueFamilyProperties> queueProperties(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(graphics->getPhysicalDevice(), &numQueueFamilies, queueProperties.data());

    bool viableDevice = false;
    for (uint32 i = 0; i < numQueueFamilies; ++i)
    {
        VkBool32 supportsPresent;
        vkGetPhysicalDeviceSurfaceSupportKHR(graphics->getPhysicalDevice(), i, surface, &supportsPresent);
        if (supportsPresent)
        {
            viableDevice = true;
            break;
        }
    }
    if (!viableDevice)
    {
        std::cerr << "Device not suitable for presenting to surface " << surface << ", use a different one" << std::endl;
    }

    recreateSwapchain(createInfo);
}

Window::~Window()
{
    vkDestroySurfaceKHR(instance, surface, nullptr);
    glfwDestroyWindow(static_cast<GLFWwindow *>(windowHandle));
}

void Window::beginFrame()
{
    glfwPollEvents();
    advanceBackBuffer();
}

void Window::endFrame()
{
    present();
}

void Window::onWindowCloseEvent()
{
}

Gfx::PTexture2D Window::getBackBuffer()
{
    return backBufferImages[currentImageIndex];
}

void Window::setKeyCallback(std::function<void(KeyCode, KeyAction, KeyModifier)> callback)
{
    keyCallback = callback;
}

void Window::advanceBackBuffer()
{
    VkResult res = VK_ERROR_OUT_OF_DATE_KHR;
    uint32 imageIndex = 0;
    const int32 prevSemaphoreIndex = semaphoreIndex;
    semaphoreIndex = (semaphoreIndex + 1) % Gfx::numFramesBuffered;

    while (res != VK_SUCCESS)
    {
        res = vkAcquireNextImageKHR(
            graphics->getDevice(),
            swapchain,
            UINT64_MAX,
            imageAcquired[semaphoreIndex]->getHandle(),
            VK_NULL_HANDLE,
            &imageIndex);
        if (res == VK_ERROR_OUT_OF_DATE_KHR)
        {
            semaphoreIndex = prevSemaphoreIndex;
        }
        if (res == VK_ERROR_SURFACE_LOST_KHR)
        {
            semaphoreIndex = prevSemaphoreIndex;
        }
    }
    imageAcquiredSemaphore = imageAcquired[semaphoreIndex];
    currentImageIndex = imageIndex;

    backBufferImages[currentImageIndex]->changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    PCmdBuffer cmdBuffer = graphics->getGraphicsCommands()->getCommands();
    graphics->getGraphicsCommands()->getCommands()->addWaitSemaphore(VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, imageAcquiredSemaphore);
    graphics->getGraphicsCommands()->submitCommands();
}

void Window::recreateSwapchain(const WindowCreateInfo &windowInfo)
{
    destroySwapchain();
    sizeX = windowInfo.width;
    sizeY = windowInfo.height;
    pixelFormat = cast(windowInfo.pixelFormat);
    bFullscreen = windowInfo.bFullscreen;

    uint32_t numFormats;
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(graphics->getPhysicalDevice(), surface, &numFormats, nullptr));
    Array<VkSurfaceFormatKHR> formats(numFormats);
    VK_CHECK(vkGetPhysicalDeviceSurfaceFormatsKHR(graphics->getPhysicalDevice(), surface, &numFormats, formats.data()));

    uint32_t numPresentModes;
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(graphics->getPhysicalDevice(), surface, &numPresentModes, nullptr));
    Array<VkPresentModeKHR> modes(numPresentModes);
    VK_CHECK(vkGetPhysicalDeviceSurfacePresentModesKHR(graphics->getPhysicalDevice(), surface, &numPresentModes, modes.data()));

    chooseSurfaceFormat(formats, windowInfo.pixelFormat);
    choosePresentMode(modes);
    createSwapchain();
}

void Window::present()
{
    backBufferImages[currentImageIndex]->changeLayout(VK_IMAGE_LAYOUT_PRESENT_SRC_KHR);
    graphics->getGraphicsCommands()->submitCommands(renderFinished[currentImageIndex]);
    VkSemaphore renderFinishedHandle = renderFinished[currentImageIndex]->getHandle();
    VkPresentInfoKHR info;
    info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    info.pNext = nullptr;
    info.swapchainCount = 1;
    info.pSwapchains = &swapchain;
    info.pImageIndices = (uint32 *)&currentImageIndex;
    info.pResults = 0;
    info.waitSemaphoreCount = 1;
    info.pWaitSemaphores = &renderFinishedHandle;

    VkResult presentResult = VK_ERROR_OUT_OF_DATE_KHR;
    // Trial and error
    while (presentResult != VK_SUCCESS)
    {
        presentResult = vkQueuePresentKHR(graphics->getGraphicsCommands()->getQueue()->getHandle(), &info);
    }
}

void Window::createSwapchain()
{
    VkSurfaceCapabilitiesKHR surfProperties;
    VK_CHECK(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(graphics->getPhysicalDevice(), surface, &surfProperties));

    uint32 desiredNumBuffers = Gfx::numFramesBuffered;

    VkExtent2D extent;
    extent.width = sizeX;
    extent.height = sizeY;
    VkSwapchainCreateInfoKHR swapchainInfo =
        init::SwapchainCreateInfo(
            surface,
            desiredNumBuffers,
            surfaceFormat.format,
            surfaceFormat.colorSpace,
            extent,
            1,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
            VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
            presentMode,
            VK_TRUE);
    VK_CHECK(vkCreateSwapchainKHR(graphics->getDevice(), &swapchainInfo, nullptr, &swapchain));

    uint32 numSwapchainImages;
    VK_CHECK(vkGetSwapchainImagesKHR(graphics->getDevice(), swapchain, &numSwapchainImages, nullptr));
    Array<VkImage> swapchainImages(numSwapchainImages);
    VK_CHECK(vkGetSwapchainImagesKHR(graphics->getDevice(), swapchain, &numSwapchainImages, swapchainImages.data()));

    PCmdBuffer cmdBuffer = graphics->getGraphicsCommands()->getCommands();

    TextureCreateInfo backBufferCreateInfo;
    backBufferCreateInfo.width = sizeX;
    backBufferCreateInfo.height = sizeY;
    backBufferCreateInfo.usage = Gfx::SE_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    backBufferCreateInfo.resourceData.owner = Gfx::QueueType::GRAPHICS;
    backBufferCreateInfo.format = cast(surfaceFormat.format);
    for (uint32 i = 0; i < numSwapchainImages; ++i)
    {
        imageAcquired[i] = new Semaphore(graphics);
        renderFinished[i] = new Semaphore(graphics);
        backBufferImages[i] = new Texture2D(graphics, backBufferCreateInfo, swapchainImages[i]);

        VkClearColorValue clearColor;
        std::memset(&clearColor, 0, sizeof(VkClearColorValue));
        backBufferImages[i]->changeLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        VkImageSubresourceRange range = init::ImageSubresourceRange(VK_IMAGE_ASPECT_COLOR_BIT);
        vkCmdClearColorImage(cmdBuffer->getHandle(), backBufferImages[i]->getHandle(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);
        backBufferImages[i]->changeLayout(VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
    }
    graphics->getGraphicsCommands()->submitCommands();
    currentImageIndex = -1;
}

void Window::destroySwapchain()
{
    if (swapchain != VK_NULL_HANDLE)
    {
        vkDestroySwapchainKHR(graphics->getDevice(), swapchain, nullptr);
    }
    for (uint32 i = 0; i < Gfx::numFramesBuffered; ++i)
    {
        imageAcquired[i] = nullptr;
    }
}

void Window::chooseSurfaceFormat(const Array<VkSurfaceFormatKHR> &available, Gfx::SeFormat preferred)
{
    VkFormat preferredFormat = cast(preferred);
    for (auto availabeFormat : available)
    {
        if (availabeFormat.format == preferredFormat)
        {
            surfaceFormat = availabeFormat;
            return;
        }
    }
    if (available.size() == 1 && available[0].format == VK_FORMAT_UNDEFINED)
    {
        surfaceFormat = {VK_FORMAT_B8G8R8A8_UNORM, VK_COLOR_SPACE_SRGB_NONLINEAR_KHR};
        return;
    }
    for (const auto &availableFormat : available)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            surfaceFormat = availableFormat;
            return;
        }
    }
    surfaceFormat = available[0];
}
void Window::choosePresentMode(const Array<VkPresentModeKHR> &modes)
{
    for (auto mode : modes)
    {
        if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            presentMode = mode;
            return;
        }
    }
    presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
}

Viewport::Viewport(PGraphics graphics, PWindow owner, const ViewportCreateInfo &viewportInfo)
    : Gfx::Viewport(owner, viewportInfo), graphics(graphics)
{
    handle.width = static_cast<float>(viewportInfo.sizeX);
    handle.height = static_cast<float>(viewportInfo.sizeY);
    handle.x = static_cast<float>(viewportInfo.offsetX);
    handle.y = static_cast<float>(viewportInfo.offsetY) + handle.height;
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