#pragma once
#include "Enums.h"
#include "Graphics/Pipeline.h"

namespace Seele
{
namespace Vulkan
{
class VertexInput : public Gfx::VertexInput
{
public:
    VertexInput(VertexInputStateCreateInfo createInfo);
    virtual ~VertexInput();
private:
};
DECLARE_REF(PipelineLayout)
DECLARE_REF(Graphics)
class GraphicsPipeline : public Gfx::GraphicsPipeline
{
public:
    GraphicsPipeline(PGraphics graphics, VkPipeline handle, Gfx::OPipelineLayout pipelineLayout);
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
    ComputePipeline(PGraphics graphics, VkPipeline handle, Gfx::OPipelineLayout pipelineLayout);
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