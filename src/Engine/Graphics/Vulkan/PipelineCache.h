#pragma once
#include "Pipeline.h"
#include "RayTracing.h"

namespace Seele {
namespace Vulkan {
class PipelineCache {
  public:
    PipelineCache(PGraphics graphics, const std::string& cacheFilePath);
    ~PipelineCache();
    PGraphicsPipeline createPipeline(Gfx::LegacyPipelineCreateInfo createInfo);
    PGraphicsPipeline createPipeline(Gfx::MeshPipelineCreateInfo createInfo);
    PComputePipeline createPipeline(Gfx::ComputePipelineCreateInfo createInfo);

    PRayTracingPipeline createPipeline(Gfx::RayTracingPipelineCreateInfo createInfo);

  private:
    Map<uint32, OGraphicsPipeline> graphicsPipelines;
    Map<uint32, OComputePipeline> computePipelines;
    Map<uint32, ORayTracingPipeline> rayTracingPipelines;
    std::mutex cacheLock;
    VkPipelineCache cache;
    PGraphics graphics;
    std::string cacheFile;
};
DEFINE_REF(PipelineCache)
} // namespace Vulkan
} // namespace Seele