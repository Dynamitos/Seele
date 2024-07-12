#pragma once
#include "Buffer.h"
#include "Descriptor.h"
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
    constexpr VkTransformMatrixKHR getTransform() const { return matrix; }
    constexpr uint64 getIndexOffset() const { return indexOffset; }
    constexpr uint64 getVertexOffset() const { return vertexOffset; }
    constexpr uint64 getVertexCount() const { return vertexCount; }
    constexpr uint32 getPrimitiveCount() const { return primitiveCount; }

  private:
    PGraphics graphics;
    VkAccelerationStructureKHR handle;
    OBufferAllocation buffer;
    PMaterialInstance material;
    VkTransformMatrixKHR matrix;
    uint64 indexOffset;
    uint64 vertexOffset;
    uint64 vertexCount;
    uint32 primitiveCount;
    friend class Graphics;
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
    friend class DescriptorSet;
};
DEFINE_REF(TopLevelAS)
class RayTracingPipeline : public Gfx::RayTracingPipeline {
  public:
    RayTracingPipeline(PGraphics graphics, VkPipeline handle, OBufferAllocation rayGen, uint64 rayGenStride, OBufferAllocation hit,
                       uint64 hitStride, OBufferAllocation miss, uint64 missStride, OBufferAllocation callable, uint64 callableStride,
                       Gfx::PPipelineLayout layout);
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
    VkStridedDeviceAddressRegionKHR getCallableRegion() {
        return VkStridedDeviceAddressRegionKHR{
            .deviceAddress = callable->deviceAddress,
            .stride = callableStride,
            .size = callable->size,
        };
    }
    VkPipelineLayout getLayout() const { return layout.cast<PipelineLayout>()->getHandle(); }

  private:
    PGraphics graphics;
    VkPipeline pipeline;
    OBufferAllocation rayGen;
    uint64 rayGenStride;
    OBufferAllocation hit;
    uint64 hitStride;
    OBufferAllocation miss;
    uint64 missStride;
    OBufferAllocation callable;
    uint64 callableStride;
};
DEFINE_REF(RayTracingPipeline)
} // namespace Vulkan
} // namespace Seele