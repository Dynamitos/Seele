#pragma once
#include "Pipeline.h"

namespace Seele
{
namespace Vulkan
{
class PipelineCache
{
public:
    PipelineCache(PGraphics graphics, const std::string& cacheFilePath);
    ~PipelineCache();
    PGraphicsPipeline createPipeline(Gfx::LegacyPipelineCreateInfo createInfo);
    PGraphicsPipeline createPipeline(Gfx::MeshPipelineCreateInfo createInfo);
    PComputePipeline createPipeline(Gfx::ComputePipelineCreateInfo createInfo);
private:
    std::map<uint32, OGraphicsPipeline> graphicsPipelines;
    Map<uint32, OComputePipeline> computePipelines;
    std::mutex cacheLock;
    VkPipelineCache cache;
    PGraphics graphics;
    std::string cacheFile;
};
DEFINE_REF(PipelineCache)
} // namespace Vulkan
} // namespace Seele