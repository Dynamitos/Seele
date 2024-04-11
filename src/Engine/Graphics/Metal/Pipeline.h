#pragma once
#include "Graphics/Initializer.h"
#include "Graphics/Pipeline.h"
#include "Resources.h"
#include "MinimalEngine.h"

namespace Seele {
namespace Metal {
class VertexInput : public Gfx::VertexInput
{
public:
  VertexInput(VertexInputStateCreateInfo createInfo);
  virtual ~VertexInput();
};
DECLARE_REF(PipelineLayout)
DECLARE_REF(Graphics)
class GraphicsPipeline : public Gfx::GraphicsPipeline
{
public:
  GraphicsPipeline(PGraphics graphics, Gfx::LegacyPipelineCreateInfo createInfo);
  GraphicsPipeline(PGraphics graphics, Gfx::MeshPipelineCreateInfo createInfo);
  virtual ~GraphicsPipeline();
private:
  MTL::RenderPipelineState* state;
};
DEFINE_REF(GraphicsPipeline)
class ComputePipeline : public Gfx::ComputePipeline
{
public:
  ComputePipeline(PGraphics graphics, Gfx::ComputePipelineCreateInfo createInfo);
  virtual ~ComputePipeline();
private:
  MTL::ComputePipelineState* state;
};
DEFINE_REF(ComputePipeline)
}
}