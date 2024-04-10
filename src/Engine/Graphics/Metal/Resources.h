#pragma once
#include "Graphics/Resources.h"
#include <Foundation/Foundation.hpp>
#include <Metal/Metal.hpp>
#include <QuartzCore/CAMetalLayer.h>
#include <QuartzCore/CAMetalLayer.hpp>
#include <QuartzCore/QuartzCore.hpp>

namespace Seele {
namespace Metal {
DECLARE_REF(Graphics);
class Fence {
public:
  Fence(PGraphics graphics);
  ~Fence();
  MTL::Fence *getHandle() const { return handle; }

private:
  MTL::Fence *handle;
};
DEFINE_REF(Fence);
class Event {
public:
  Event(PGraphics graphics);
  ~Event();
  MTL::Event *getHandle() const { return handle; }

private:
  MTL::Event *handle;
};
DEFINE_REF(Event)
} // namespace Metal
} // namespace Seele