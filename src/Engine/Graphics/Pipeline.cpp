#include "Pipeline.h"

using namespace Seele;
using namespace Seele::Gfx;

GraphicsPipeline::GraphicsPipeline(OPipelineLayout layout) 
    : layout(std::move(layout))
{
}

GraphicsPipeline::~GraphicsPipeline()
{
}

PPipelineLayout GraphicsPipeline::getPipelineLayout() const
{
    return layout;
}

ComputePipeline::ComputePipeline(OPipelineLayout layout)
    : layout(std::move(layout))
{
}

ComputePipeline::~ComputePipeline()
{
}

PPipelineLayout ComputePipeline::getPipelineLayout() const
{
    return layout;
}
