#pragma once
#include "VulkanGraphicsResources.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(PipelineLayout)
DECLARE_REF(Graphics)
class GraphicsPipeline : public Gfx::GraphicsPipeline
{
public:
    GraphicsPipeline(PGraphics graphics, VkPipeline handle, PPipelineLayout pipelineLayout, const GraphicsPipelineCreateInfo& createInfo);
    virtual ~GraphicsPipeline();
    void bind(VkCommandBuffer handle);
    VkPipelineLayout getLayout() const;
private:
    PGraphics graphics;
    VkPipeline pipeline;
};
DEFINE_REF(GraphicsPipeline)
class ComputePipeline : public Gfx::ComputePipeline
{
public:
    ComputePipeline(PGraphics graphics, VkPipeline handle, PPipelineLayout pipelineLayout, const ComputePipelineCreateInfo& createInfo);
    virtual ~ComputePipeline();
    void bind(VkCommandBuffer handle);
    VkPipelineLayout getLayout() const;
private:
    PGraphics graphics;
    VkPipeline pipeline;
};
DEFINE_REF(ComputePipeline)
} // namespace Vulkan
} // namespace Seele