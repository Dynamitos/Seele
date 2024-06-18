#pragma once
#include "Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/RayTracing.h"
#include <vulkan/vulkan_core.h>
#include "Buffer.h"

namespace Seele {
namespace Vulkan {
class BottomLevelAS : public Gfx::BottomLevelAS {
  public:
    BottomLevelAS(PGraphics graphics, const Gfx::BottomLevelASCreateInfo& createInfo);
    ~BottomLevelAS();

  private:
    PGraphics graphics;
    VkAccelerationStructureKHR handle;
    OBufferAllocation buffer;

};
DEFINE_REF(BottomLevelAS)
class TopLevelAS : public Gfx::TopLevelAS {
  public:
    TopLevelAS(PGraphics graphics, const Gfx::TopLevelASCreateInfo& createInfo);
    ~TopLevelAS();

  private:
    PGraphics graphics;
    VkAccelerationStructureKHR handle;
};
DEFINE_REF(TopLevelAS)

class RayTracingPipeline : public Gfx::RayTracingPipeline
{
  public:
  private:
};
DEFINE_REF(RayTracingPipeline)
} // namespace Vulkan
} // namespace Seele