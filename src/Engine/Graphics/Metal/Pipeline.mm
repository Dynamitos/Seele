#include "Pipeline.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Graphics.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Resources.h"

using namespace Seele;
using namespace Seele::Metal;

VertexInput::VertexInput(VertexInputStateCreateInfo createInfo) : Gfx::VertexInput(createInfo) {}

VertexInput::~VertexInput() {}

GraphicsPipeline::GraphicsPipeline(PGraphics graphics, MTL::PrimitiveType primitive, MTL::RenderPipelineState* pipeline,
                                   Gfx::OPipelineLayout createInfo)
    : Gfx::GraphicsPipeline(std::move(createInfo)), graphics(graphics), state(pipeline), primitiveType(primitive) {}

GraphicsPipeline::~GraphicsPipeline() {}

ComputePipeline::ComputePipeline(PGraphics graphics, MTL::ComputePipelineState* pipeline,
                                 Gfx::OPipelineLayout createInfo)
    : Gfx::ComputePipeline(std::move(createInfo)), graphics(graphics), state(pipeline) {}

ComputePipeline::~ComputePipeline() {}
