#include "Pipeline.h"
#include "Descriptor.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

VertexInput::VertexInput(VertexInputStateCreateInfo createInfo) : Gfx::VertexInput(createInfo) {}

VertexInput::~VertexInput() {}

GraphicsPipeline::GraphicsPipeline(PGraphics graphics, VkPipeline handle, Gfx::PPipelineLayout pipelineLayout)
    : Gfx::GraphicsPipeline(std::move(pipelineLayout)), graphics(graphics), pipeline(handle) {}

GraphicsPipeline::~GraphicsPipeline() {}

void GraphicsPipeline::bind(VkCommandBuffer handle) { vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline); }

VkPipelineLayout GraphicsPipeline::getLayout() const { return Gfx::PPipelineLayout(layout).cast<PipelineLayout>()->getHandle(); }

ComputePipeline::ComputePipeline(PGraphics graphics, VkPipeline handle, Gfx::PPipelineLayout pipelineLayout)
    : Gfx::ComputePipeline(std::move(pipelineLayout)), graphics(graphics), pipeline(handle) {}

ComputePipeline::~ComputePipeline() {}

void ComputePipeline::bind(VkCommandBuffer handle) { vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline); }

VkPipelineLayout ComputePipeline::getLayout() const { return Gfx::PPipelineLayout(layout).cast<PipelineLayout>()->getHandle(); }