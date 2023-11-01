#pragma once
#include "Enums.h"
#include "Graphics/Graphics.h"

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
DECLARE_REF(Window)
DECLARE_REF(RenderTargetLayout)
DECLARE_REF(Viewport)
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
    virtual Gfx::OWindow createWindow(const WindowCreateInfo &createInfo) override;
    virtual Gfx::OViewport createViewport(Gfx::PWindow owner, const ViewportCreateInfo &createInfo) override;

    virtual Gfx::ORenderPass createRenderPass(Gfx::ORenderTargetLayout layout, Gfx::PViewport renderArea) override;
    virtual void beginRenderPass(Gfx::PRenderPass renderPass) override;
    virtual void endRenderPass() override;

    virtual void executeCommands(const Array<Gfx::PRenderCommand>& commands) override;
    virtual void executeCommands(const Array<Gfx::PComputeCommand>& commands) override;

    virtual Gfx::OTexture2D createTexture2D(const TextureCreateInfo &createInfo) override;
    virtual Gfx::OTexture3D createTexture3D(const TextureCreateInfo &createInfo) override;
    virtual Gfx::OTextureCube createTextureCube(const TextureCreateInfo &createInfo) override;
    virtual Gfx::OUniformBuffer createUniformBuffer(const UniformBufferCreateInfo &bulkData) override;
    virtual Gfx::OShaderBuffer createShaderBuffer(const ShaderBufferCreateInfo &bulkData) override;
    virtual Gfx::OVertexBuffer createVertexBuffer(const VertexBufferCreateInfo &bulkData) override;
    virtual Gfx::OIndexBuffer createIndexBuffer(const IndexBufferCreateInfo &bulkData) override;
    
    virtual Gfx::PRenderCommand createRenderCommand(const std::string& name) override;
    virtual Gfx::PComputeCommand createComputeCommand(const std::string& name) override;
    
    virtual Gfx::OVertexDeclaration createVertexDeclaration(const Array<Gfx::VertexElement>& element) override;
    virtual Gfx::OVertexShader createVertexShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OComputeShader createComputeShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OTaskShader createTaskShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OMeshShader createMeshShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OGraphicsPipeline createGraphicsPipeline(const Gfx::LegacyPipelineCreateInfo& createInfo) override;
    virtual Gfx::OGraphicsPipeline createGraphicsPipeline(const Gfx::MeshPipelineCreateInfo& createInfo) override;
    virtual Gfx::OComputePipeline createComputePipeline(const Gfx::ComputePipelineCreateInfo& createInfo) override;
    virtual Gfx::OSamplerState createSamplerState(const SamplerCreateInfo& createInfo) override;

    virtual Gfx::ODescriptorLayout createDescriptorLayout(const std::string& name = "") override;
    virtual Gfx::OPipelineLayout createPipelineLayout(Gfx::PPipelineLayout baseLayout = nullptr) override;

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

    OQueue graphicsQueue;
    OQueue computeQueue;
    OQueue transferQueue;
    OQueue dedicatedTransferQueue;
    OPipelineCache pipelineCache;
    std::mutex renderPassLock;
    PRenderPass activeRenderPass;
    PFramebuffer activeFramebuffer;
    thread_local static OCommandBufferManager graphicsCommands;
    thread_local static OCommandBufferManager computeCommands;
    thread_local static OCommandBufferManager transferCommands;
    thread_local static OCommandBufferManager dedicatedTransferCommands;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures features;
    VkDebugReportCallbackEXT callback;
    std::mutex viewportLock;
    Array<PViewport> viewports;
    std::mutex allocatedFrameBufferLock;
    Map<uint32, PFramebuffer> allocatedFramebuffers;
    OAllocator allocator;
    OStagingManager stagingManager;

    friend class Window;
};
DEFINE_REF(Graphics)
} // namespace Vulkan
} // namespace Seele