#pragma once
#include "VulkanGraphicsResources.h"
#include "Graphics/Graphics.h"
#include "NsightAftermathGpuCrashTracker.h"

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(Allocator)
DECLARE_REF(StagingManager)
DECLARE_REF(CommandBufferManager)
DECLARE_REF(Queue)
DECLARE_REF(RenderPass)
DECLARE_REF(Framebuffer)
DECLARE_REF(RenderCommand)
DECLARE_REF(PipelineCache)
class Graphics : public Gfx::Graphics
{
public:
    Graphics();
    virtual ~Graphics();
    constexpr VkInstance getInstance() const { return instance; };
    constexpr VkDevice getDevice() const { return handle; };
    constexpr VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; };

    PCommandBufferManager getQueueCommands(Gfx::QueueType queueType);
    PCommandBufferManager getGraphicsCommands();
    PCommandBufferManager getComputeCommands();
    PCommandBufferManager getTransferCommands();
    PCommandBufferManager getDedicatedTransferCommands();

    PAllocator getAllocator();
    PStagingManager getStagingManager();

    // Inherited via Graphics
    virtual void init(GraphicsInitializer initializer) override;
    virtual Gfx::PWindow createWindow(const WindowCreateInfo &createInfo) override;
    virtual Gfx::PViewport createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo) override;

    virtual Gfx::PRenderPass createRenderPass(Gfx::PRenderTargetLayout layout, Gfx::PViewport renderArea) override;
    virtual void beginRenderPass(Gfx::PRenderPass renderPass) override;
    virtual void endRenderPass() override;

    virtual void executeCommands(const Array<Gfx::PRenderCommand>& commands) override;
    virtual void executeCommands(const Array<Gfx::PComputeCommand>& commands) override;

    virtual Gfx::PTexture2D createTexture2D(const TextureCreateInfo &createInfo) override;
    virtual Gfx::PUniformBuffer createUniformBuffer(const UniformBufferCreateInfo &bulkData) override;
    virtual Gfx::PStructuredBuffer createStructuredBuffer(const StructuredBufferCreateInfo &bulkData) override;
    virtual Gfx::PVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) override;
    virtual Gfx::PIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) override;
    virtual Gfx::PRenderCommand createRenderCommand(const std::string& name) override;
    virtual Gfx::PComputeCommand createComputeCommand(const std::string& name) override;
    virtual Gfx::PVertexDeclaration createVertexDeclaration(const Array<Gfx::VertexElement>& element) override;
    virtual Gfx::PVertexShader createVertexShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PControlShader createControlShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PEvaluationShader createEvaluationShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PGeometryShader createGeometryShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PComputeShader createComputeShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(const GraphicsPipelineCreateInfo& createInfo) override;
    virtual Gfx::PComputePipeline createComputePipeline(const ComputePipelineCreateInfo& createInfo) override;
    virtual Gfx::PSamplerState createSamplerState(const SamplerCreateInfo& createInfo) override;

    virtual Gfx::PDescriptorLayout createDescriptorLayout(const std::string& name = "") override;
    virtual Gfx::PPipelineLayout createPipelineLayout(Gfx::PPipelineLayout baseLayout = nullptr) override;

    virtual void copyTexture(Gfx::PTexture srcTexture, Gfx::PTexture dstTexture) override;
protected:
    Array<const char *> getRequiredExtensions();
    void initInstance(GraphicsInitializer initInfo);
    void setupDebugCallback();
    void pickPhysicalDevice();
    void createDevice(GraphicsInitializer initInfo);

    VkInstance instance;
    VkDevice handle;
    VkPhysicalDevice physicalDevice;

    PQueue graphicsQueue;
    PQueue computeQueue;
    PQueue transferQueue;
    PQueue dedicatedTransferQueue;
    PPipelineCache pipelineCache;
    std::mutex renderPassLock;
    PRenderPass activeRenderPass;
    PFramebuffer activeFramebuffer;
    thread_local static PCommandBufferManager graphicsCommands;
    thread_local static PCommandBufferManager computeCommands;
    thread_local static PCommandBufferManager transferCommands;
    thread_local static PCommandBufferManager dedicatedTransferCommands;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkDebugReportCallbackEXT callback;
    std::mutex viewportLock;
    Array<PViewport> viewports;
    std::mutex allocatedFrameBufferLock;
    Map<uint32, PFramebuffer> allocatedFramebuffers;
    PAllocator allocator;
    PStagingManager stagingManager;
    GpuCrashTracker crashTracker;

    friend class Window;
};
DEFINE_REF(Graphics)
} // namespace Vulkan
} // namespace Seele