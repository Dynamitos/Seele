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
private:
    VkPipelineCache cache;
    PGraphics graphics;
    std::string cacheFile;
};
DEFINE_REF(PipelineCache);
} // namespace Vulkan
} // namespace Seele