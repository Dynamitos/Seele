#pragma once
#include "VulkanGraphicsResources.h"
#include "Graphics/Graphics.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Allocator);
DECLARE_REF(StagingManager);
DECLARE_REF(CommandBufferManager);
DECLARE_REF(Queue);
DECLARE_REF(Framebuffer);
DECLARE_REF(RenderCommand);
DECLARE_REF(PipelineCache);
class Graphics : public Gfx::Graphics
{
public:
	Graphics();
	virtual ~Graphics();
	VkInstance getInstance() const;
	VkDevice getDevice() const;
	VkPhysicalDevice getPhysicalDevice() const;

	PCommandBufferManager getGraphicsCommands();
	PCommandBufferManager getComputeCommands();
	PCommandBufferManager getTransferCommands();
	PCommandBufferManager getDedicatedTransferCommands();

	const QueueFamilyMapping getFamilyMapping() const
	{
		return queueMapping;
	}
	QueueOwnedResourceDeletion &getDeletionQueue()
	{
		return deletionQueue;
	}

	PAllocator getAllocator();
	PStagingManager getStagingManager();

	// Inherited via Graphics
	virtual void init(GraphicsInitializer initializer) override;
	virtual Gfx::PWindow createWindow(const WindowCreateInfo &createInfo) override;
	virtual Gfx::PViewport createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo) override;

	virtual Gfx::PRenderPass createRenderPass(Gfx::PRenderTargetLayout layout) override;
	virtual void beginRenderPass(Gfx::PRenderPass renderPass) override;
	virtual void endRenderPass() override;

	virtual void executeCommands(Array<Gfx::PRenderCommand> commands) override;

	virtual Gfx::PTexture2D createTexture2D(const TextureCreateInfo &createInfo) override;
	virtual Gfx::PUniformBuffer createUniformBuffer(const BulkResourceData &bulkData) override;
	virtual Gfx::PStructuredBuffer createStructuredBuffer(const BulkResourceData &bulkData) override;
	virtual Gfx::PVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) override;
	virtual Gfx::PIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) override;
	virtual Gfx::PRenderCommand createRenderCommand() override;
	virtual Gfx::PGraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;

protected:
	Array<const char *> getRequiredExtensions();
	void initInstance(GraphicsInitializer initInfo);
	void setupDebugCallback();
	void pickPhysicalDevice();
	void createDevice(GraphicsInitializer initInfo);

	VkDevice handle;
	VkPhysicalDevice physicalDevice;
	VkInstance instance;

	PQueue graphicsQueue;
	PQueue computeQueue;
	PQueue transferQueue;
	PQueue dedicatedTransferQueue;
	QueueFamilyMapping queueMapping;
	QueueOwnedResourceDeletion deletionQueue;
	PPipelineCache pipelineCache;
	PCommandBufferManager graphicsCommands;
	PCommandBufferManager computeCommands;
	PCommandBufferManager transferCommands;
	PCommandBufferManager dedicatedTransferCommands;
	VkPhysicalDeviceProperties props;
	VkPhysicalDeviceFeatures features;
	VkDebugReportCallbackEXT callback;
	Array<PViewport> viewports;
	Map<uint32, PFramebuffer> allocatedFramebuffers;
	PAllocator allocator;
	PStagingManager stagingManager;

	friend class Window;
};
DEFINE_REF(Graphics);
} // namespace Vulkan
} // namespace Seele