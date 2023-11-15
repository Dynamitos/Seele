#pragma once
#include "MinimalEngine.h"
#include "Initializer.h"
#include "Resources.h"
#include "Containers/Array.h"

namespace Seele
{
namespace Gfx
{
DECLARE_REF(Window)
DECLARE_REF(Viewport)
DECLARE_REF(RenderTargetLayout)
DECLARE_REF(ShaderCompiler)
DECLARE_REF(Texture)
DECLARE_REF(Texture2D)
DECLARE_REF(Texture3D)
DECLARE_REF(TextureCube)
DECLARE_REF(DescriptorLayout)
DECLARE_REF(VertexShader)
DECLARE_REF(FragmentShader)
DECLARE_REF(ComputeShader)
DECLARE_REF(TaskShader)
DECLARE_REF(MeshShader)
DECLARE_REF(ShaderBuffer)
DECLARE_REF(VertexBuffer)
DECLARE_REF(IndexBuffer)
DECLARE_REF(UniformBuffer)
DECLARE_REF(PipelineLayout)
DECLARE_REF(GraphicsPipeline)
DECLARE_REF(ComputePipeline)
DECLARE_REF(RenderCommand)
DECLARE_REF(ComputeCommand)
class Graphics
{
public:
    Graphics();
    virtual ~Graphics();
    virtual void init(GraphicsInitializer initializer) = 0;
    
  	const QueueFamilyMapping getFamilyMapping() const
  	{
  		return queueMapping;
  	}

    PShaderCompiler getShaderCompiler()
    {
        return shaderCompiler;
    }
    
    virtual OWindow createWindow(const WindowCreateInfo &createInfo) = 0;
    virtual OViewport createViewport(PWindow owner, const ViewportCreateInfo &createInfo) = 0;

    virtual ORenderPass createRenderPass(ORenderTargetLayout layout, PViewport renderArea) = 0;
    virtual void beginRenderPass(PRenderPass renderPass) = 0;
    virtual void endRenderPass() = 0;

    virtual void executeCommands(const Array<PRenderCommand>& commands) = 0;
    virtual void executeCommands(const Array<PComputeCommand>& commands) = 0;

    virtual OTexture2D createTexture2D(const TextureCreateInfo &createInfo) = 0;
    virtual OTexture3D createTexture3D(const TextureCreateInfo &createInfo) = 0;
    virtual OTextureCube createTextureCube(const TextureCreateInfo &createInfo) = 0;
    virtual OUniformBuffer createUniformBuffer(const UniformBufferCreateInfo &bulkData) = 0;
    virtual OShaderBuffer createShaderBuffer(const ShaderBufferCreateInfo &bulkData) = 0;
    virtual OVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) = 0;
    virtual OIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) = 0;
    
    virtual PRenderCommand createRenderCommand(const std::string& name = "") = 0;
    virtual PComputeCommand createComputeCommand(const std::string& name = "") = 0;
    
    virtual OVertexShader createVertexShader(const ShaderCreateInfo& createInfo) = 0;
    virtual OFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) = 0;
    virtual OComputeShader createComputeShader(const ShaderCreateInfo& createInfo) = 0;
    virtual OMeshShader createMeshShader(const ShaderCreateInfo& createInfo) = 0;
    virtual OTaskShader createTaskShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PGraphicsPipeline createGraphicsPipeline(LegacyPipelineCreateInfo createInfo) = 0;
    virtual PGraphicsPipeline createGraphicsPipeline(MeshPipelineCreateInfo createInfo) = 0;
    virtual PComputePipeline createComputePipeline(ComputePipelineCreateInfo createInfo) = 0;
    virtual OSampler createSampler(const SamplerCreateInfo& createInfo) = 0;

  	virtual ODescriptorLayout createDescriptorLayout(const std::string& name = "") = 0;
  	virtual OPipelineLayout createPipelineLayout(PPipelineLayout baseLayout = nullptr) = 0;

    bool supportMeshShading() const { return meshShadingEnabled; }
protected:
    QueueFamilyMapping queueMapping;
    OShaderCompiler shaderCompiler;
    bool meshShadingEnabled = false;
    friend class Window;
};
DEFINE_REF(Graphics)
} // namespace Gfx
} // namespace Seele