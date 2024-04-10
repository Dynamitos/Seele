#include "Graphics.h"
#include "RenderPass.h"
#include "Command.h"
#include "Window.h"

using namespace Seele;
using namespace Seele::Metal;

Graphics::Graphics()
{
  
}

Graphics::~Graphics()
{
  
}

void Graphics::init(GraphicsInitializer)
{
  device = MTL::CreateSystemDefaultDevice();
  library = device->newDefaultLibrary();
  assert(library);
  queue = new CommandQueue(this);
}

Gfx::OWindow Graphics::createWindow(const WindowCreateInfo &createInfo)
{
  return new Window(this, createInfo);
}

Gfx::OViewport Graphics::createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo)
{
  return new Viewport(owner, createInfo);
}

Gfx::ORenderPass Graphics::createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport renderArea)
{
  return new RenderPass(this, layout, dependencies, renderArea);
}

void Graphics::beginRenderPass(Gfx::PRenderPass renderPass)
{
  queue->getCommands()->beginRenderPass(renderPass.cast<RenderPass>());
}

void Graphics::endRenderPass()
{
  queue->getCommands()->endRenderPass();
}

void Graphics::waitDeviceIdle()
{
  queue->getCommands()->waitDeviceIdle();
}

void Graphics::executeCommands(const Array<Gfx::PRenderCommand>& commands)
{
  queue->getCommands()->executeCommands(commands);  
}

void Graphics::executeCommands(const Array<Gfx::PComputeCommand>& commands)
{
  queue->getCommands()->executeCommands(commands);
}

Gfx::OTexture2D Graphics::createTexture2D(const TextureCreateInfo &createInfo)
{
  return new Texture2D(this, createInfo);
}
Gfx::OTexture3D Graphics::createTexture3D(const TextureCreateInfo &createInfo)
{
  
  return new Texture3D(this, createInfo);
}
Gfx::OTextureCube Graphics::createTextureCube(const TextureCreateInfo &createInfo)
{
  return new TextureCube(this, createInfo);
}
Gfx::OUniformBuffer Graphics::createUniformBuffer(const UniformBufferCreateInfo &bulkData)
{
  
}
Gfx::OShaderBuffer Graphics::createShaderBuffer(const ShaderBufferCreateInfo &bulkData)
{
  
}
Gfx::OVertexBuffer Graphics::createVertexBuffer(const VertexBufferCreateInfo &bulkData)
{
  
}
Gfx::OIndexBuffer Graphics::createIndexBuffer(const IndexBufferCreateInfo &bulkData)
{
  
}

Gfx::PRenderCommand Graphics::createRenderCommand(const std::string& name)
{
  
}
Gfx::PComputeCommand Graphics::createComputeCommand(const std::string& name)
{
  
}

Gfx::OVertexShader Graphics::createVertexShader(const ShaderCreateInfo& createInfo)
{
  
}
Gfx::OFragmentShader Graphics::createFragmentShader(const ShaderCreateInfo& createInfo)
{
  
}
Gfx::OComputeShader Graphics::createComputeShader(const ShaderCreateInfo& createInfo)
{
  
}
Gfx::OMeshShader Graphics::createMeshShader(const ShaderCreateInfo& createInfo)
{
  
}
Gfx::OTaskShader Graphics::createTaskShader(const ShaderCreateInfo& createInfo)
{
  
}
Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo)
{
  
}
Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo)
{
  
}
Gfx::PComputePipeline Graphics::createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo)
{
  
}
Gfx::OSampler Graphics::createSampler(const SamplerCreateInfo& createInfo)
{
  
}

Gfx::ODescriptorLayout Graphics::createDescriptorLayout(const std::string& name)
{
  
}
Gfx::OPipelineLayout Graphics::createPipelineLayout(Gfx::PPipelineLayout baseLayout)
{
  
}

Gfx::OVertexInput Graphics::createVertexInput(VertexInputStateCreateInfo createInfo)
{
  
}

void Graphics::resolveTexture(Gfx::PTexture source, Gfx::PTexture destination)
{
  
}

