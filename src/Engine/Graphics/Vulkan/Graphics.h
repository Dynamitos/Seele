#pragma once
#include "Enums.h"
#include "Graphics/Graphics.h"
#include <vk_mem_alloc.h>

namespace Seele
{
namespace Vulkan
{
DECLARE_REF(DestructionManager)
DECLARE_REF(CommandPool)
DECLARE_REF(Queue)
DECLARE_REF(PipelineCache)
DECLARE_REF(Framebuffer)
class Graphics : public Gfx::Graphics
{
public:
    Graphics();
    virtual ~Graphics();
    constexpr VkInstance getInstance() const { return instance; };
    constexpr VkDevice getDevice() const { return handle; };
    constexpr VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; };

    PCommandPool getQueueCommands(Gfx::QueueType queueType);
    PCommandPool getGraphicsCommands();
    PCommandPool getComputeCommands();
    PCommandPool getTransferCommands();
    PCommandPool getDedicatedTransferCommands();

    VmaAllocator getAllocator();
    PDestructionManager getDestructionManager();

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
    
    virtual Gfx::OVertexShader createVertexShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OComputeShader createComputeShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OTaskShader createTaskShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OMeshShader createMeshShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo) override;
    virtual Gfx::PComputePipeline createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) override;
    virtual Gfx::OSampler createSampler(const SamplerCreateInfo& createInfo) override;

    virtual Gfx::ODescriptorLayout createDescriptorLayout(const std::string& name = "") override;
    virtual Gfx::OPipelineLayout createPipelineLayout(Gfx::PPipelineLayout baseLayout = nullptr) override;

    virtual Gfx::OVertexInput createVertexInput(VertexInputStateCreateInfo createInfo) override;

    virtual void resolveTexture(Gfx::PTexture source, Gfx::PTexture destination) override;

    void vkCmdDrawMeshTasksEXT(VkCommandBuffer handle, uint32 groupX, uint32 groupY, uint32 groupZ);

protected:
    PFN_vkCmdDrawMeshTasksEXT cmdDrawMeshTasks;
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
    thread_local static PCommandPool graphicsCommands;
    thread_local static PCommandPool computeCommands;
    thread_local static PCommandPool transferCommands;
    thread_local static PCommandPool dedicatedTransferCommands;
    std::mutex poolLock;
    Array<OCommandPool> pools;
    VkPhysicalDeviceProperties props;
    VkPhysicalDeviceFeatures2 features;
    VkPhysicalDeviceVulkan11Features features11;
    VkPhysicalDeviceVulkan12Features features12;
    VkDebugUtilsMessengerEXT callback;
    Map<uint32, OFramebuffer> allocatedFramebuffers;
    VmaAllocator allocator;
    OPipelineCache pipelineCache;
    ODestructionManager destructionManager;

    friend class Window;
};
DEFINE_REF(Graphics)
} // namespace Vulkan
} // namespace Seele