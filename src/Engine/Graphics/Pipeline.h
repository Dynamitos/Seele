#pragma once
#include "Enums.h"
#include "Descriptor.h"

namespace Seele
{
namespace Gfx
{
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