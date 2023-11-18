#pragma once
#include "Initializer.h"
#include "Texture.h"

namespace Seele
{
namespace Gfx
{
class Window
{
public:
    Window();
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
    virtual void setResizeCallback(std::function<void(uint32, uint32)> callback) = 0;
    constexpr SeFormat getSwapchainFormat() const
    {
        return framebufferFormat;
    }
    constexpr SeSampleCountFlags getNumSamples() const
    {
        return sampleFlags;
    }
    constexpr uint32 getFramebufferWidth() const
    {
        return framebufferWidth;
    }
    constexpr uint32 getFramebufferHeight() const
    {
        return framebufferHeight;
    }

protected:
    SeFormat framebufferFormat;
    SeSampleCountFlags sampleFlags;
    uint32 framebufferWidth;
    uint32 framebufferHeight;
};
DEFINE_REF(Window)

class Viewport
{
public:
    Viewport(PWindow owner, const ViewportCreateInfo& createInfo);
    virtual ~Viewport();
    virtual void resize(uint32 newX, uint32 newY) = 0;
    virtual void move(uint32 newOffsetX, uint32 newOffsetY) = 0;
    constexpr PWindow getOwner() const { return owner; }
    constexpr uint32 getWidth() const { return sizeX; }
    constexpr uint32 getHeight() const { return sizeY; }
    constexpr uint32 getOffsetX() const { return offsetX; }
    constexpr uint32 getOffsetY() const { return offsetY; }
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

}
}