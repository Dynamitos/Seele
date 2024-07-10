#pragma once
#include "Buffer.h"
#include "Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/RayTracing.h"
#include <vulkan/vulkan_core.h>

namespace Seele {
namespace Vulkan {
class BottomLevelAS : public Gfx::BottomLevelAS {
  public:
    BottomLevelAS(PGraphics graphics, const Gfx::BottomLevelASCreateInfo& createInfo);
    ~BottomLevelAS();
    uint64 getDeviceAddress() const { return buffer->deviceAddress; }

  private:
    PGraphics graphics;
    VkAccelerationStructureKHR handle;
    OBufferAllocation buffer;
    PMaterialInstance material;
};
DEFINE_REF(BottomLevelAS)
class TopLevelAS : public Gfx::TopLevelAS {
  public:
    TopLevelAS(PGraphics graphics, const Gfx::TopLevelASCreateInfo& createInfo);
    ~TopLevelAS();
    const VkAccelerationStructureKHR getHandle() const { return handle; }

  private:
    PGraphics graphics;
    VkAccelerationStructureKHR handle;
    OBufferAllocation instanceAllocation;
    OBufferAllocation buffer;
};
DEFINE_REF(TopLevelAS)

class RayTracingPipeline : public Gfx::RayTracingPipeline {
  public:
    RayTracingPipeline(PGraphics graphics, VkPipeline handle, OBufferAllocation rayGen, uint64 rayGenStride, OBufferAllocation hit,
                       uint64 hitStride, OBufferAllocation miss, uint64 missStride, Gfx::PPipelineLayout layout);
    virtual ~RayTracingPipeline();
    void bind(VkCommandBuffer handle);
    VkStridedDeviceAddressRegionKHR getRayGenRegion() {
        return VkStridedDeviceAddressRegionKHR{
            .deviceAddress = rayGen->deviceAddress,
            .stride = rayGenStride,
            .size = rayGen->size,
        };
    }
    VkStridedDeviceAddressRegionKHR getHitRegion() {
        return VkStridedDeviceAddressRegionKHR{
            .deviceAddress = hit->deviceAddress,
            .stride = hitStride,
            .size = hit->size,
        };
    }
    VkStridedDeviceAddressRegionKHR getMissRegion() {
        return VkStridedDeviceAddressRegionKHR{
            .deviceAddress = miss->deviceAddress,
            .stride = missStride,
            .size = miss->size,
        };
    }
  private:
    PGraphics graphics;
    VkPipeline pipeline;
    OBufferAllocation rayGen;
    uint64 rayGenStride;
    OBufferAllocation hit;
    uint64 hitStride;
    OBufferAllocation miss;
    uint64 missStride;
};
DEFINE_REF(RayTracingPipeline)
} // namespace Vulkan
} // namespace Seele