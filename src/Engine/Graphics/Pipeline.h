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
    GraphicsPipeline(PPipelineLayout layout);
    virtual ~GraphicsPipeline();
    PPipelineLayout getPipelineLayout() const;
protected:
    PPipelineLayout layout;
};
DEFINE_REF(GraphicsPipeline)

class ComputePipeline
{
public:
    ComputePipeline(PPipelineLayout layout);
    virtual ~ComputePipeline();
    PPipelineLayout getPipelineLayout() const;
protected:
    PPipelineLayout layout;
};
DEFINE_REF(ComputePipeline)
}
}
