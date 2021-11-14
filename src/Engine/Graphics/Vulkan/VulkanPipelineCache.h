#pragma once
#include "VulkanPipeline.h"

namespace Seele
{
namespace Vulkan
{
class PipelineCache
{
public:
    PipelineCache(PGraphics graphics, const std::string& cacheFilePath);
    ~PipelineCache();
    PGraphicsPipeline createPipeline(const GraphicsPipelineCreateInfo& createInfo);
    PComputePipeline createPipeline(const ComputePipelineCreateInfo& createInfo);
private:
    VkPipelineCache cache;
    PGraphics graphics;
    std::string cacheFile;
    std::mutex createdPipelinesLock;
    Map<uint32, VkPipeline> createdPipelines;
};
DEFINE_REF(PipelineCache)
} // namespace Vulkan
} // namespace Seele