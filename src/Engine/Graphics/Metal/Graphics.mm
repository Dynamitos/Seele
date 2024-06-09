#include "Graphics.h"
#include "Buffer.h"
#include "Command.h"
#include "Graphics/Initializer.h"
#include "Graphics/Metal/Descriptor.h"
#include "Graphics/Metal/Pipeline.h"
#include "Graphics/Metal/PipelineCache.h"
#include "Graphics/Metal/Resources.h"
#include "RenderPass.h"
#include "Resources.h"
#include "Shader.h"
#include "Window.h"


using namespace Seele;
using namespace Seele::Metal;

Graphics::Graphics() {}

Graphics::~Graphics() {}

void Graphics::init(GraphicsInitializer) {
  glfwInit();
  device = MTL::CreateSystemDefaultDevice();
  queue = new CommandQueue(this);
  ioQueue = new IOCommandQueue(this);
  cache = new PipelineCache(this, "pipelines.metal");
  meshShadingEnabled = false;
}

Gfx::OWindow Graphics::createWindow(const WindowCreateInfo &createInfo) {
  return new Window(this, createInfo);
}

Gfx::OViewport Graphics::createViewport(Gfx::PWindow owner,
                                        const ViewportCreateInfo &createInfo) {
  return new Viewport(owner, createInfo);
}

Gfx::ORenderPass
Graphics::createRenderPass(Gfx::RenderTargetLayout layout,
                           Array<Gfx::SubPassDependency> dependencies,
                           Gfx::PViewport renderArea) {
  return new RenderPass(this, layout, dependencies, renderArea);
}

void Graphics::beginRenderPass(Gfx::PRenderPass renderPass) {
  queue->getCommands()->beginRenderPass(renderPass.cast<RenderPass>());
}

void Graphics::endRenderPass() { queue->getCommands()->endRenderPass(); }

void Graphics::waitDeviceIdle() { queue->getCommands()->waitDeviceIdle(); }

void Graphics::executeCommands(Array<Gfx::ORenderCommand> commands) {
  queue->executeCommands(std::move(commands));
}

void Graphics::executeCommands(Array<Gfx::OComputeCommand> commands) {
  queue->executeCommands(std::move(commands));
}

Gfx::OTexture2D Graphics::createTexture2D(const TextureCreateInfo &createInfo) {
  return new Texture2D(this, createInfo);
}

Gfx::OTexture3D Graphics::createTexture3D(const TextureCreateInfo &createInfo) {

  return new Texture3D(this, createInfo);
}

Gfx::OTextureCube
Graphics::createTextureCube(const TextureCreateInfo &createInfo) {
  return new TextureCube(this, createInfo);
}

Gfx::OUniformBuffer
Graphics::createUniformBuffer(const UniformBufferCreateInfo &bulkData) {
  return new UniformBuffer(this, bulkData);
}

Gfx::OShaderBuffer
Graphics::createShaderBuffer(const ShaderBufferCreateInfo &bulkData) {
  return new ShaderBuffer(this, bulkData);
}

Gfx::OVertexBuffer
Graphics::createVertexBuffer(const VertexBufferCreateInfo &bulkData) {
  return new VertexBuffer(this, bulkData);
}

Gfx::OIndexBuffer
Graphics::createIndexBuffer(const IndexBufferCreateInfo &bulkData) {
  return new IndexBuffer(this, bulkData);
}

Gfx::ORenderCommand Graphics::createRenderCommand(const std::string &name) {
  return queue->getRenderCommand(name);
}

Gfx::OComputeCommand Graphics::createComputeCommand(const std::string &name) {
  return queue->getComputeCommand(name);
}

Gfx::OVertexShader
Graphics::createVertexShader(const ShaderCreateInfo &createInfo) {
  OVertexShader result = new VertexShader(this);
  result->create(createInfo);
  return result;
}

Gfx::OFragmentShader
Graphics::createFragmentShader(const ShaderCreateInfo &createInfo) {
  OFragmentShader result = new FragmentShader(this);
  result->create(createInfo);
  return result;
}

Gfx::OComputeShader
Graphics::createComputeShader(const ShaderCreateInfo &createInfo) {
  OComputeShader result = new ComputeShader(this);
  result->create(createInfo);
  return result;
}

Gfx::OMeshShader
Graphics::createMeshShader(const ShaderCreateInfo &createInfo) {
  OMeshShader result = new MeshShader(this);
  result->create(createInfo);
  return result;
}

Gfx::OTaskShader
Graphics::createTaskShader(const ShaderCreateInfo &createInfo) {
  OTaskShader result = new TaskShader(this);
  result->create(createInfo);
  return result;
}

Gfx::PGraphicsPipeline
Graphics::createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo) {
  return cache->createPipeline(std::move(createInfo));
}

Gfx::PGraphicsPipeline
Graphics::createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo) {
  return cache->createPipeline(std::move(createInfo));
}

Gfx::PComputePipeline
Graphics::createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) {
  return cache->createPipeline(std::move(createInfo));
}

Gfx::OSampler Graphics::createSampler(const SamplerCreateInfo &createInfo) {
  return new Sampler(this, createInfo);
}

Gfx::ODescriptorLayout
Graphics::createDescriptorLayout(const std::string &name) {
  return new DescriptorLayout(this, name);
}

Gfx::OPipelineLayout
Graphics::createPipelineLayout(const std::string &name,
                               Gfx::PPipelineLayout baseLayout) {
  return new PipelineLayout(this, name, baseLayout);
}

Gfx::OVertexInput
Graphics::createVertexInput(VertexInputStateCreateInfo createInfo) {
  return new VertexInput(createInfo);
}

void Graphics::resolveTexture(Gfx::PTexture source, Gfx::PTexture destination) {
}
