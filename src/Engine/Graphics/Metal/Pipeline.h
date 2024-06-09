#pragma once
#include "Graphics/Initializer.h"
#include "Graphics/Pipeline.h"
#include "MinimalEngine.h"
#include "Resources.h"

namespace Seele {
namespace Metal {
class VertexInput : public Gfx::VertexInput {
  public:
    VertexInput(VertexInputStateCreateInfo createInfo);
    virtual ~VertexInput();
};
DECLARE_REF(PipelineLayout)
DECLARE_REF(Graphics)
class GraphicsPipeline : public Gfx::GraphicsPipeline {
  public:
    GraphicsPipeline(PGraphics graphics, MTL::PrimitiveType primitive, MTL::RenderPipelineState* pipeline, Gfx::PPipelineLayout createInfo);
    virtual ~GraphicsPipeline();
    constexpr MTL::RenderPipelineState* getHandle() const { return state; }
    constexpr MTL::PrimitiveType getPrimitive() const { return primitiveType; }
    PGraphics graphics;
    MTL::RenderPipelineState* state;
    MTL::PrimitiveType primitiveType;

  private:
};
DEFINE_REF(GraphicsPipeline)
class ComputePipeline : public Gfx::ComputePipeline {
  public:
    ComputePipeline(PGraphics graphics, MTL::ComputePipelineState* pipeline, Gfx::PPipelineLayout);
    virtual ~ComputePipeline();
    constexpr MTL::ComputePipelineState* getHandle() const { return state; }

    PGraphics graphics;

  private:
    MTL::ComputePipelineState* state;
};
DEFINE_REF(ComputePipeline)
} // namespace Metal
} // namespace Seele
