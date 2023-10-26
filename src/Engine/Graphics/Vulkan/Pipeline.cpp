#include "Pipeline.h"
#include "DescriptorSets.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Vulkan;

GraphicsPipeline::GraphicsPipeline(PGraphics graphics, VkPipeline handle, PPipelineLayout pipelineLayout)
    : Gfx::GraphicsPipeline(pipelineLayout)
    , graphics(graphics)
    , pipeline(handle)
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

void GraphicsPipeline::bind(VkCommandBuffer handle)
{
    vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
}

VkPipelineLayout GraphicsPipeline::getLayout() const
{
    return layout.cast<PipelineLayout>()->getHandle();  
}

ComputePipeline::ComputePipeline(PGraphics graphics, VkPipeline handle, PPipelineLayout pipelineLayout) 
    : Gfx::ComputePipeline(pipelineLayout)
    , graphics(graphics)
    , pipeline(handle)
{
    
}

ComputePipeline::~ComputePipeline() 
{
}

void ComputePipeline::bind(VkCommandBuffer handle) 
{
    vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);
}

VkPipelineLayout ComputePipeline::getLayout() const
{
    return layout.cast<PipelineLayout>()->getHandle();
}