#pragma once
#include "Metal/Metal.hpp"
#include "Graphics/Graphics.h"

namespace Seele
{
namespace Metal
{
class Graphics : public Gfx::Graphics
{
public:
    Graphics();
    virtual ~Graphics();
    virtual void init(GraphicsInitializer initializer) = 0;
    
    virtual Gfx::OWindow createWindow(const WindowCreateInfo &createInfo) = 0;
    virtual Gfx::OViewport createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo) = 0;

    virtual Gfx::ORenderPass createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport renderArea) = 0;
    virtual void beginRenderPass(Gfx::PRenderPass renderPass) = 0;
    virtual void endRenderPass() = 0;
    virtual void waitDeviceIdle() = 0;

    virtual void executeCommands(const Array<Gfx::PRenderCommand>& commands) = 0;
    virtual void executeCommands(const Array<Gfx::PComputeCommand>& commands) = 0;

    virtual Gfx::OTexture2D createTexture2D(const TextureCreateInfo &createInfo) = 0;
    virtual Gfx::OTexture3D createTexture3D(const TextureCreateInfo &createInfo) = 0;
    virtual Gfx::OTextureCube createTextureCube(const TextureCreateInfo &createInfo) = 0;
    virtual Gfx::OUniformBuffer createUniformBuffer(const UniformBufferCreateInfo &bulkData) = 0;
    virtual Gfx::OShaderBuffer createShaderBuffer(const ShaderBufferCreateInfo &bulkData) = 0;
    virtual Gfx::OVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) = 0;
    virtual Gfx::OIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) = 0;
    
    virtual Gfx::PRenderCommand createRenderCommand(const std::string& name = "") = 0;
    virtual Gfx::PComputeCommand createComputeCommand(const std::string& name = "") = 0;
    
    virtual Gfx::OVertexShader createVertexShader(const ShaderCreateInfo& createInfo) = 0;
    virtual Gfx::OFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) = 0;
    virtual Gfx::OComputeShader createComputeShader(const ShaderCreateInfo& createInfo) = 0;
    virtual Gfx::OMeshShader createMeshShader(const ShaderCreateInfo& createInfo) = 0;
    virtual Gfx::OTaskShader createTaskShader(const ShaderCreateInfo& createInfo) = 0;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo) = 0;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo) = 0;
    virtual Gfx::PComputePipeline createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) = 0;
    virtual Gfx::OSampler createSampler(const SamplerCreateInfo& createInfo) = 0;

  	virtual Gfx::ODescriptorLayout createDescriptorLayout(const std::string& name = "") = 0;
  	virtual Gfx::OPipelineLayout createPipelineLayout(Gfx::PPipelineLayout baseLayout = nullptr) = 0;

    virtual Gfx::OVertexInput createVertexInput(VertexInputStateCreateInfo createInfo) = 0;

    virtual void resolveTexture(Gfx::PTexture source, Gfx::PTexture destination) = 0;
protected:
    MTL::Device* device;
};
} // namespace Metal  
} // namespace Seele
