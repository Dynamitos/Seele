#include "Window.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Enums.h"
#include "Graphics/Texture.h"
#include "Command.h"
#include "Metal/MTLTexture.hpp"
using namespace Seele;
using namespace Seele::Metal;

double currentFrameDelta = 0;
double Gfx::getCurrentFrameDelta()
{
    return currentFrameDelta;
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

Window::Window(PGraphics graphics, const WindowCreateInfo& createInfo)
    : graphics(graphics)
    , preferences(createInfo)
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

    metalWindow = glfwGetCocoaWindow(handle);
    metalLayer = [CAMetalLayer layer];
    metalLayer.device = (__bridge id<MTLDevice>)graphics->getDevice();
    metalLayer.pixelFormat = MTLPixelFormatBGRA8Unorm;    
    metalLayer.drawableSize = CGSizeMake(createInfo.width, createInfo.height);
    metalWindow.contentView.layer = metalLayer;
    metalWindow.contentView.wantsLayer = YES;
}

Window::~Window()
{
    glfwDestroyWindow(static_cast<GLFWwindow *>(windowHandle));
}

void Window::pollInput()
{
    glfwPollEvents();
}

void Window::beginFrame()
{
    @autoreleasepool {
        drawable = (__bridge CA::MetalDrawable*)[metalLayer nextDrawable];
        MTL::Texture* buf = drawable->texture();
        backBuffer = new Texture2D(graphics, TextureCreateInfo {
            .width = static_cast<uint32>(buf->width()),
            .height = static_cast<uint32>(buf->height()),
            .depth = static_cast<uint32>(buf->depth()),
            .elements = static_cast<uint32>(buf->arrayLength()),
            .mipLevels = static_cast<uint32>(buf->mipmapLevelCount()),
            .format = cast(buf->pixelFormat()),
            .usage = MTL::TextureUsageRenderTarget,
            .samples = static_cast<uint32>(buf->sampleCount()),
        }, buf);
    }
}

void Window::endFrame()
{
    graphics->getQueue()->getCommands()->present(drawable);
}

void Window::onWindowCloseEvent()
{
}

Gfx::PTexture2D Window::getBackBuffer() const
{
    return PTexture2D(backBuffer);
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
    if(width == 0 || height == 0)
    {
        paused = true;
        return;
    }
    paused = true;
    resizeCallback(width, height);
}

Viewport::Viewport(PWindow owner, const ViewportCreateInfo& createInfo)
    : Gfx::Viewport(owner, createInfo)
{
    viewport.width = sizeX;
    viewport.height = sizeY;
    viewport.originX = offsetX;
    viewport.originY = offsetY;
    viewport.znear = 0.0f;
    viewport.zfar = 1.0f;
}

Viewport::~Viewport()
{

}

void Viewport::resize(uint32 newX, uint32 newY)
{
    viewport.width = newX;
    viewport.height = newY;
}

void Viewport::move(uint32 newOffset, uint32 newOffsetY)
{
    viewport.originX = newOffset;
    viewport.originY = newOffsetY;
}