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
    virtual void init(GraphicsInitializer initializer) override;
    
    virtual Gfx::OWindow createWindow(const WindowCreateInfo &createInfo) override;
    virtual Gfx::OViewport createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo) override;

    virtual Gfx::ORenderPass createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport renderArea) override;
    virtual void beginRenderPass(Gfx::PRenderPass renderPass) override;
    virtual void endRenderPass() override;
    virtual void waitDeviceIdle() override;

    virtual void executeCommands(const Array<Gfx::PRenderCommand>& commands) override;
    virtual void executeCommands(const Array<Gfx::PComputeCommand>& commands) override;

    virtual Gfx::OTexture2D createTexture2D(const TextureCreateInfo &createInfo) override;
    virtual Gfx::OTexture3D createTexture3D(const TextureCreateInfo &createInfo) override;
    virtual Gfx::OTextureCube createTextureCube(const TextureCreateInfo &createInfo) override;
    virtual Gfx::OUniformBuffer createUniformBuffer(const UniformBufferCreateInfo &bulkData) override;
    virtual Gfx::OShaderBuffer createShaderBuffer(const ShaderBufferCreateInfo &bulkData) override;
    virtual Gfx::OVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) override;
    virtual Gfx::OIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) override;
    
    virtual Gfx::PRenderCommand createRenderCommand(const std::string& name = "") override;
    virtual Gfx::PComputeCommand createComputeCommand(const std::string& name = "") override;
    
    virtual Gfx::OVertexShader createVertexShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OComputeShader createComputeShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OMeshShader createMeshShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OTaskShader createTaskShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo) override;
    virtual Gfx::PComputePipeline createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) override;
    virtual Gfx::OSampler createSampler(const SamplerCreateInfo& createInfo) override;

  	virtual Gfx::ODescriptorLayout createDescriptorLayout(const std::string& name = "") override;
  	virtual Gfx::OPipelineLayout createPipelineLayout(Gfx::PPipelineLayout baseLayout = nullptr) override;

    virtual Gfx::OVertexInput createVertexInput(VertexInputStateCreateInfo createInfo) override;

    virtual void resolveTexture(Gfx::PTexture source, Gfx::PTexture destination) override;
    
    MTL::Device* getDevice() const { return device; }
protected:
    MTL::Device* device;
    MTL::Library* library;
    MTL::CommandQueue* queue;
};
DEFINE_REF(Graphics)
} // namespace Metal  
} // namespace Seele
