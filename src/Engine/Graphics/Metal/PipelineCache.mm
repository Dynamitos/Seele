#include "PipelineCache.h"
#include "Descriptor.h"
#include "Graphics.h"
#include "Shader.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Metal/MTLRenderPipeline.hpp"
#include "Metal/MTLVertexDescriptor.hpp"

using namespace Seele;
using namespace Seele::Metal;

PipelineCache::PipelineCache(PGraphics graphics, const std::string& name) : graphics(graphics), cacheFile(name) {}

PipelineCache::~PipelineCache() {}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::LegacyPipelineCreateInfo createInfo) {
  PPipelineLayout layout = Gfx::PPipelineLayout(createInfo.pipelineLayout).cast<PipelineLayout>();
  uint32 hash = layout->getHash();
  MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();

  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();

  MTL::VertexAttributeDescriptorArray* attributes = vertexDescriptor->attributes();

  const auto& vertexInfo = createInfo.vertexInput->getInfo();

  for (size_t attr = 0; attr < vertexInfo.attributes.size(); ++attr) {
    MTL::VertexAttributeDescriptor* attribute = MTL::VertexAttributeDescriptor::alloc()->init();
    attribute->setBufferIndex(vertexInfo.attributes[attr].binding);
    switch (vertexInfo.attributes[attr].format) {
    case Gfx::SE_FORMAT_R32G32B32_SFLOAT:
      attribute->setFormat(MTL::VertexFormatFloat3);
      break;
    default:
      throw std::logic_error("TODO");
    }
    attribute->setOffset(vertexInfo.attributes[attr].offset);
    attributes->setObject(attribute, attr);
  }

  MTL::VertexBufferLayoutDescriptorArray* bufferLayout = vertexDescriptor->layouts();

  for (size_t binding = 0; binding < vertexInfo.bindings.size(); ++binding) {
    MTL::VertexBufferLayoutDescriptor* buffer = MTL::VertexBufferLayoutDescriptor::alloc()->init();
    buffer->setStride(vertexInfo.bindings[binding].stride);
    buffer->setStepRate(1);
    switch (vertexInfo.bindings[binding].inputRate) {
    case Gfx::SE_VERTEX_INPUT_RATE_VERTEX:
      buffer->setStepFunction(MTL::VertexStepFunctionPerVertex);
      break;
    case Gfx::SE_VERTEX_INPUT_RATE_INSTANCE:
      buffer->setStepFunction(MTL::VertexStepFunctionPerInstance);
      break;
    }
    bufferLayout->setObject(buffer, binding);
  }

  pipelineDescriptor->setVertexDescriptor(vertexDescriptor);

  pipelineDescriptor->setVertexFunction(createInfo.vertexShader.cast<VertexShader>()->getFunction());
  

  vertexDescriptor->release();
  pipelineDescriptor->release();
}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::MeshPipelineCreateInfo createInfo) {}

PComputePipeline PipelineCache::createPipeline(Gfx::ComputePipelineCreateInfo createInfo) {}
