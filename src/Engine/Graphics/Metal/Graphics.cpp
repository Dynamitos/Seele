#include "Graphics.h"
#include "Metal/MTLDevice.hpp"
#include "Metal/MTLRenderCommandEncoder.hpp"
#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "Foundation/Foundation.hpp"
#include "Metal/Metal.hpp"
#include "QuartzCore/QuartzCore.hpp"

using namespace Seele;
using namespace Seele::Metal;

Graphics::Graphics()
{
  
}
Graphics::~Graphics()
{
  
}
void Graphics::init(GraphicsInitializer initializer)
{
  device = MTL::CreateSystemDefaultDevice();
}

Gfx::OWindow Graphics::createWindow(const WindowCreateInfo &createInfo)
{
  
}
Gfx::OViewport Graphics::createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo)
{
  
}

Gfx::ORenderPass Graphics::createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport renderArea)
{
  
}
void Graphics::beginRenderPass(Gfx::PRenderPass renderPass)
{
  
}
void Graphics::endRenderPass()
{
  
}
void Graphics::waitDeviceIdle()
{
  
}

void Graphics::executeCommands(const Array<Gfx::PRenderCommand>& commands)
{
  
}
void Graphics::executeCommands(const Array<Gfx::PComputeCommand>& commands)
{
  
}

Gfx::OTexture2D Graphics::createTexture2D(const TextureCreateInfo &createInfo)
{
  
}
Gfx::OTexture3D Graphics::createTexture3D(const TextureCreateInfo &createInfo)
{
  
}
Gfx::OTextureCube Graphics::createTextureCube(const TextureCreateInfo &createInfo)
{
  
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


