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
    PGraphicsPipeline createPipeline(const Gfx::LegacyPipelineCreateInfo& createInfo);
    PGraphicsPipeline createPipeline(const Gfx::MeshPipelineCreateInfo& createInfo);
    PComputePipeline createPipeline(const Gfx::ComputePipelineCreateInfo& createInfo);
private:
    VkPipelineCache cache;
    PGraphics graphics;
    std::string cacheFile;
};
DEFINE_REF(PipelineCache)
} // namespace Vulkan
} // namespace Seele