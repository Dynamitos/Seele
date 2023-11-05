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
    OGraphicsPipeline createPipeline(const Gfx::LegacyPipelineCreateInfo& createInfo);
    OGraphicsPipeline createPipeline(const Gfx::MeshPipelineCreateInfo& createInfo);
    OComputePipeline createPipeline(const Gfx::ComputePipelineCreateInfo& createInfo);
private:
    VkPipelineCache cache;
    PGraphics graphics;
    std::string cacheFile;
};
DEFINE_REF(PipelineCache)
} // namespace Vulkan
} // namespace Seele