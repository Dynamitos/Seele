#pragma once
#include "Foundation/NSAutoreleasePool.hpp"
#include "Graphics.h"
#include "Graphics/Window.h"
#include "Resources.h"
#include "Texture.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_COCOA
#include <GLFW/glfw3native.h>

#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace Seele {
namespace Metal {
class Window : public Gfx::Window {
public:
  Window(PGraphics graphics, const WindowCreateInfo& createInfo);
  virtual ~Window();
  virtual void pollInput() override;
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

private:
  void createBackBuffer();

  PGraphics graphics;
  WindowCreateInfo preferences;
  GLFWwindow* windowHandle;
  NSWindow* metalWindow;
  CAMetalLayer* metalLayer;
  CA::MetalDrawable* drawable;
  OTexture2D backBuffer;

  std::function<void(KeyCode, InputAction, KeyModifier)> keyCallback;
  std::function<void(double, double)> mouseMoveCallback;
  std::function<void(MouseButton, InputAction, KeyModifier)> mouseButtonCallback;
  std::function<void(double, double)> scrollCallback;
  std::function<void(int, const char**)> fileCallback;
  std::function<void()> closeCallback;
  std::function<void(uint32, uint32)> resizeCallback;
};
DEFINE_REF(Window);
class Viewport : public Gfx::Viewport {
public:
  Viewport(PWindow owner, const ViewportCreateInfo& createInfo);
  virtual ~Viewport();
  constexpr MTL::Viewport getHandle() const { return viewport; }
  virtual void resize(uint32 newX, uint32 newY);
  virtual void move(uint32 newOffset, uint32 newOffsetY);

private:
  MTL::Viewport viewport;
};
} // namespace Metal
} // namespace Seele
