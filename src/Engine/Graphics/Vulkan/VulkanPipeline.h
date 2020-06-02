#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(PipelineLayout);
DECLARE_REF(Graphics);
class GraphicsPipeline : public Gfx::GraphicsPipeline
{
public:
    GraphicsPipeline(PGraphics graphics, VkPipeline handle, PPipelineLayout pipelineLayout, const GraphicsPipelineCreateInfo& createInfo);
    virtual ~GraphicsPipeline();
    void bind(VkCommandBuffer handle);
    VkPipelineLayout getLayout() const;
private:
    VkPipeline pipeline;
    PGraphics graphics;
};
DEFINE_REF(GraphicsPipeline);
} // namespace Vulkan
} // namespace Seele