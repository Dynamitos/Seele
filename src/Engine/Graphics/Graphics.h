#pragma once
#include "MinimalEngine.h"
#include "GraphicsResources.h"
#include "Containers/Array.h"
#include "VertexShaderInput.h"
#include "ShaderCompiler.h"

namespace Seele
{
namespace Gfx
{
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

    PShaderCompiler getShaderCompiler() const
    {
        return shaderCompiler;
    }
    
    virtual PWindow createWindow(const WindowCreateInfo &createInfo) = 0;
    virtual PViewport createViewport(PWindow owner, const ViewportCreateInfo &createInfo) = 0;

    virtual PRenderPass createRenderPass(PRenderTargetLayout layout) = 0;
    virtual void beginRenderPass(PRenderPass renderPass) = 0;
    virtual void endRenderPass() = 0;

    virtual void executeCommands(const Array<PRenderCommand>& commands) = 0;
    virtual void executeCommands(const Array<PComputeCommand>& commands) = 0;

    virtual PTexture2D createTexture2D(const TextureCreateInfo &createInfo) = 0;
    virtual PUniformBuffer createUniformBuffer(const UniformBufferCreateInfo &bulkData) = 0;
    virtual PStructuredBuffer createStructuredBuffer(const StructuredBufferCreateInfo &bulkData) = 0;
    virtual PVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) = 0;
    virtual PIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) = 0;
    virtual PRenderCommand createRenderCommand(const std::string& name = "") = 0;
    virtual PComputeCommand createComputeCommand(const std::string& name = "") = 0;
    virtual PVertexDeclaration createVertexDeclaration(const Array<VertexElement>& element) = 0;
    virtual PVertexShader createVertexShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PControlShader createControlShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PEvaluationShader createEvaluationShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PGeometryShader createGeometryShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PComputeShader createComputeShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PGraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;
    virtual PComputePipeline createComputePipeline(const ComputePipelineCreateInfo& createInfo) = 0;
    virtual PSamplerState createSamplerState(const SamplerCreateInfo& createInfo) = 0;

	virtual PDescriptorLayout createDescriptorLayout(const std::string& name = "") = 0;
	virtual PPipelineLayout createPipelineLayout() = 0;

	virtual void copyTexture(Gfx::PTexture srcTexture, Gfx::PTexture dstTexture) = 0;
    
    PVertexBuffer getNullVertexBuffer();

protected:
    PVertexBuffer nullVertexBuffer;
    QueueFamilyMapping queueMapping;
    PShaderCompiler shaderCompiler;
    friend class Window;
};
DEFINE_REF(Graphics)
} // namespace Gfx
} // namespace Seele