#pragma once
#include "Enums.h"
#include "Descriptor.h"

namespace Seele
{
namespace Gfx
{
class VertexInput
{
public:
    VertexInput(VertexInputStateCreateInfo createInfo);
    virtual ~VertexInput();
    const VertexInputStateCreateInfo& getInfo() const { return createInfo; }
private:
    VertexInputStateCreateInfo createInfo;
};
class GraphicsPipeline
{
public:
    GraphicsPipeline(OPipelineLayout layout);
    virtual ~GraphicsPipeline();
    PPipelineLayout getPipelineLayout() const;
protected:
    OPipelineLayout layout;
};
DEFINE_REF(GraphicsPipeline)

class ComputePipeline
{
public:
    ComputePipeline(OPipelineLayout layout);
    virtual ~ComputePipeline();
    PPipelineLayout getPipelineLayout() const;
protected:
    OPipelineLayout layout;
};
DEFINE_REF(ComputePipeline)
}
}