#include "Pipeline.h"

using namespace Seele;
using namespace Seele::Gfx;

VertexInput::VertexInput(VertexInputStateCreateInfo createInfo)
    : createInfo(createInfo)
{
}

VertexInput::~VertexInput()
{
}

GraphicsPipeline::GraphicsPipeline(PPipelineLayout layout)
    : layout(layout)
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

PPipelineLayout GraphicsPipeline::getPipelineLayout() const
{
    return layout;
}

ComputePipeline::ComputePipeline(PPipelineLayout layout)
    : layout(layout)
{
}

ComputePipeline::~ComputePipeline()
{
}

PPipelineLayout ComputePipeline::getPipelineLayout() const
{
    return layout;
}
