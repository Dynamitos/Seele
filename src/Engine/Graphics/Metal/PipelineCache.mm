#include "PipelineCache.h"
#include "Descriptor.h"
#include "Enums.h"
#include "Foundation/NSError.hpp"
#include "Graphics.h"
#include "Graphics/Descriptor.h"
#include "Graphics/Enums.h"
#include "Graphics/Metal/Pipeline.h"
#include "Metal/MTLComputePipeline.hpp"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#include "Metal/MTLRenderPipeline.hpp"
#include "Metal/MTLVertexDescriptor.hpp"
#include "Shader.h"
#include "Texture.h"

using namespace Seele;
using namespace Seele::Metal;

PipelineCache::PipelineCache(PGraphics graphics, const std::string& name) : graphics(graphics), cacheFile(name) {}

PipelineCache::~PipelineCache() {}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::LegacyPipelineCreateInfo createInfo) {
  PPipelineLayout layout = Gfx::PPipelineLayout(createInfo.pipelineLayout).cast<PipelineLayout>();

  MTL::RenderPipelineDescriptor* pipelineDescriptor = MTL::RenderPipelineDescriptor::alloc()->init();

  MTL::VertexDescriptor* vertexDescriptor = MTL::VertexDescriptor::alloc()->init();
  MTL::VertexAttributeDescriptorArray* attributes = vertexDescriptor->attributes();
  if(createInfo.vertexInput != nullptr)
  {
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
  }
  pipelineDescriptor->setVertexDescriptor(vertexDescriptor);

  pipelineDescriptor->setVertexFunction(createInfo.vertexShader.cast<VertexShader>()->getFunction());
  if (createInfo.fragmentShader != nullptr) {
    pipelineDescriptor->setFragmentFunction(createInfo.fragmentShader.cast<FragmentShader>()->getFunction());
  }
  pipelineDescriptor->setInputPrimitiveTopology(cast(createInfo.topology));
  if (createInfo.renderPass->getLayout().depthAttachment.getTexture() != nullptr) {
    pipelineDescriptor->setDepthAttachmentPixelFormat(
        cast(createInfo.renderPass->getLayout().depthAttachment.getTexture().cast<Texture2D>()->getFormat()));
  }
  pipelineDescriptor->setAlphaToCoverageEnabled(createInfo.multisampleState.alphaCoverageEnable);
  pipelineDescriptor->setAlphaToOneEnabled(createInfo.multisampleState.alphaToOneEnable);
  pipelineDescriptor->setRasterSampleCount(createInfo.multisampleState.samples);
  pipelineDescriptor->setRasterizationEnabled(!createInfo.rasterizationState.rasterizerDiscardEnable);

  uint32 hash = pipelineDescriptor->hash();

  if (graphicsPipelines.contains(hash)) {
    return graphicsPipelines[hash];
  }
  MTL::PrimitiveType type = MTL::PrimitiveTypeTriangle;
  switch (createInfo.topology) {
  case Gfx::SE_PRIMITIVE_TOPOLOGY_POINT_LIST:
    type = MTL::PrimitiveTypePoint;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST:
    type = MTL::PrimitiveTypeLine;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_STRIP:
    type = MTL::PrimitiveTypeLineStrip;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
    type = MTL::PrimitiveTypeTriangle;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
    type = MTL::PrimitiveTypeTriangleStrip;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
    type = MTL::PrimitiveTypeTriangle;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY:
    type = MTL::PrimitiveTypeLine;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY:
    type = MTL::PrimitiveTypeLineStrip;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
    type = MTL::PrimitiveTypeTriangle;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
    type = MTL::PrimitiveTypeTriangleStrip;
    break;
  case Gfx::SE_PRIMITIVE_TOPOLOGY_PATCH_LIST:
    type = MTL::PrimitiveTypeTriangle;
    break;
  }

  NS::Error* error;
  graphicsPipelines[hash] =
      new GraphicsPipeline(graphics, type, graphics->getDevice()->newRenderPipelineState(pipelineDescriptor, &error),
                           std::move(createInfo.pipelineLayout));
  assert(!error);

  vertexDescriptor->release();
  pipelineDescriptor->release();
  return graphicsPipelines[hash];
}

PGraphicsPipeline PipelineCache::createPipeline(Gfx::MeshPipelineCreateInfo createInfo) {
  MTL::MeshRenderPipelineDescriptor* pipelineDescriptor = MTL::MeshRenderPipelineDescriptor::alloc()->init();

  pipelineDescriptor->setMeshFunction(createInfo.meshShader.cast<VertexShader>()->getFunction());
  if (createInfo.taskShader != nullptr) {
    pipelineDescriptor->setObjectFunction(createInfo.taskShader.cast<TaskShader>()->getFunction());
  }
  if (createInfo.fragmentShader != nullptr) {
    pipelineDescriptor->setFragmentFunction(createInfo.fragmentShader.cast<FragmentShader>()->getFunction());
  }
  if (createInfo.renderPass->getLayout().depthAttachment.getTexture() != nullptr) {
    pipelineDescriptor->setDepthAttachmentPixelFormat(
        cast(createInfo.renderPass->getLayout().depthAttachment.getTexture().cast<Texture2D>()->getFormat()));
  }
  pipelineDescriptor->setAlphaToCoverageEnabled(createInfo.multisampleState.alphaCoverageEnable);
  pipelineDescriptor->setAlphaToOneEnabled(createInfo.multisampleState.alphaToOneEnable);
  pipelineDescriptor->setRasterSampleCount(createInfo.multisampleState.samples);
  pipelineDescriptor->setRasterizationEnabled(!createInfo.rasterizationState.rasterizerDiscardEnable);

  uint32 hash = pipelineDescriptor->hash();

  if (graphicsPipelines.contains(hash)) {
    return graphicsPipelines[hash];
  }

  graphics->getDevice()->newRenderPipelineState(
      pipelineDescriptor, MTL::PipelineOptionNone,
      [&](MTL::RenderPipelineState* state, MTL::RenderPipelineReflection*, NS::Error* error) {
        assert(!error);
        graphicsPipelines[hash] =
            new GraphicsPipeline(graphics, MTL::PrimitiveTypeLine, state, std::move(createInfo.pipelineLayout));
      });

  pipelineDescriptor->release();
  return graphicsPipelines[hash];
}

PComputePipeline PipelineCache::createPipeline(Gfx::ComputePipelineCreateInfo createInfo) {
  PComputeShader shader = createInfo.computeShader.cast<ComputeShader>();
  uint32 hash = shader->getShaderHash();
  if (computePipelines.contains(hash)) {
    return computePipelines[hash];
  }

  NS::Error* error;
  computePipelines[hash] =
      new ComputePipeline(graphics, graphics->getDevice()->newComputePipelineState(shader->getFunction(), &error),
                          std::move(createInfo.pipelineLayout));
  assert(!error);
  return computePipelines[hash];
}
