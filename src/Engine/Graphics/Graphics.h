#pragma once
#include "MinimalEngine.h"
#include "GraphicsResources.h"
#include "Containers/Array.h"
#include "VertexShaderInput.h"

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
    
    virtual PWindow createWindow(const WindowCreateInfo &createInfo) = 0;
    virtual PViewport createViewport(PWindow owner, const ViewportCreateInfo &createInfo) = 0;

    virtual PRenderPass createRenderPass(PRenderTargetLayout layout) = 0;
    virtual void beginRenderPass(PRenderPass renderPass) = 0;
    virtual void endRenderPass() = 0;

    virtual void executeCommands(Array<PRenderCommand> commands) = 0;

    virtual PTexture2D createTexture2D(const TextureCreateInfo &createInfo) = 0;
    virtual PUniformBuffer createUniformBuffer(const BulkResourceData &bulkData) = 0;
    virtual PStructuredBuffer createStructuredBuffer(const BulkResourceData &bulkData) = 0;
    virtual PVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) = 0;
    virtual PIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) = 0;
    virtual PRenderCommand createRenderCommand() = 0;
    virtual PVertexShader createVertexShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PControlShader createControlShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PEvaluationShader createEvaluationShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PGeometryShader createGeometryShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) = 0;
    virtual PGraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) = 0;
    
	virtual Gfx::PDescriptorLayout createDescriptorLayout() = 0;
	virtual Gfx::PPipelineLayout createPipelineLayout() = 0;

protected:
    QueueFamilyMapping queueMapping;
    friend class Window;
};
DEFINE_REF(Graphics);
} // namespace Gfx
} // namespace Seele