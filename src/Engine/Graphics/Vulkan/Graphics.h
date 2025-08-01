#pragma once
#include "Graphics/Graphics.h"
#include <vk_mem_alloc.h>

namespace Seele {
namespace Vulkan {
DECLARE_REF(DestructionManager)
DECLARE_REF(CommandPool)
DECLARE_REF(Queue)
DECLARE_REF(PipelineCache)
DECLARE_REF(Framebuffer)

template <typename T>
concept chainable_struct = requires(T t) {
    t.pNext;
    t.sType;
};
template <chainable_struct T, VkStructureType struct_type> struct Helper {};

template <typename...> struct is_unique : std::true_type {};

template <typename T, typename... Rest>
struct is_unique<T, Rest...> : std::bool_constant<(!std::same_as<T, Rest> && ... && is_unique<Rest...>::value)> {};

template <typename... T>
concept unique_chainable_structs = (chainable_struct<T> && ...) && is_unique<T...>::value;

template <typename... T> struct StructChain;
template <chainable_struct Base, VkStructureType struct_type> struct StructChain<Helper<Base, struct_type>> {
    StructChain() {
        std::memset(&b, 0, sizeof(Base));
        b.sType = struct_type;
        b.pNext = nullptr;
    }
    Base b;
    template <chainable_struct Query>
    Query& get()
        requires std::same_as<Query, Base>
    {
        return b;
    }
    template <chainable_struct Query>
    const Query& get() const
        requires std::same_as<Query, Base>
    {
        return b;
    }
    Base& get() { return b; }
    const Base& get() const { return b; }
};
template <chainable_struct This, VkStructureType struct_type, typename... Right>
struct StructChain<Helper<This, struct_type>, Right...> : StructChain<Right...> {
    StructChain() {
        std::memset(&t, 0, sizeof(This));
        t.sType = struct_type;
        t.pNext = &StructChain<Right...>::template get();
    }
    This t;
    template <chainable_struct Query> constexpr Query& get() {
        if constexpr (std::same_as<Query, This>)
            return t;
        else
            return StructChain<Right...>::template get<Query>();
    }
    template <chainable_struct Query> constexpr const Query& get() const {
        if constexpr (std::same_as<Query, This>)
            return t;
        else
            return StructChain<Right...>::template get<Query>();
    }
    constexpr This& get() { return t; }
    constexpr const This& get() const { return t; }
};

class Graphics : public Gfx::Graphics {
  public:
    Graphics();
    virtual ~Graphics();
    constexpr VkInstance getInstance() const { return instance; }
    constexpr VkDevice getDevice() const { return handle; }
    constexpr VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    constexpr VkPhysicalDeviceAccelerationStructurePropertiesKHR getAccelerationProperties() const {
        return props.get<VkPhysicalDeviceAccelerationStructurePropertiesKHR>();
    }
    constexpr VkPhysicalDeviceRayTracingPipelinePropertiesKHR getRayTracingProperties() const {
        return props.get<VkPhysicalDeviceRayTracingPipelinePropertiesKHR>();
    }
    constexpr float getTimestampPeriod() const { return props.get<VkPhysicalDeviceProperties2>().properties.limits.timestampPeriod; }
    constexpr uint64 getTimestampValidBits() const { return graphicsProps.timestampValidBits; }

    PCommandPool getQueueCommands(Gfx::QueueType queueType);
    PCommandPool getGraphicsCommands();
    PCommandPool getComputeCommands();
    PCommandPool getTransferCommands();

    VmaAllocator getAllocator() const;
    PDestructionManager getDestructionManager();

    // Inherited via Graphics
    virtual void init(GraphicsInitializer initializer) override;
    virtual Gfx::OWindow createWindow(const WindowCreateInfo& createInfo) override;
    virtual Gfx::OViewport createViewport(Gfx::PWindow owner, const ViewportCreateInfo& createInfo) override;

    virtual Gfx::ORenderPass createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, URect renderArea,
                                              std::string name, Array<uint32> viewMasks, Array<uint32> correlationMasks) override;
    virtual void beginRenderPass(Gfx::PRenderPass renderPass) override;
    virtual void endRenderPass() override;
    virtual void waitDeviceIdle() override;

    virtual void executeCommands(Gfx::ORenderCommand commands) override;
    virtual void executeCommands(Array<Gfx::ORenderCommand> commands) override;
    virtual void executeCommands(Gfx::OComputeCommand commands) override;
    virtual void executeCommands(Array<Gfx::OComputeCommand> commands) override;

    virtual Gfx::OTexture2D createTexture2D(const TextureCreateInfo& createInfo) override;
    virtual Gfx::OTexture3D createTexture3D(const TextureCreateInfo& createInfo) override;
    virtual Gfx::OTextureCube createTextureCube(const TextureCreateInfo& createInfo) override;
    virtual Gfx::OUniformBuffer createUniformBuffer(const UniformBufferCreateInfo& bulkData) override;
    virtual Gfx::OShaderBuffer createShaderBuffer(const ShaderBufferCreateInfo& bulkData) override;
    virtual Gfx::OVertexBuffer createVertexBuffer(const VertexBufferCreateInfo& bulkData) override;
    virtual Gfx::OIndexBuffer createIndexBuffer(const IndexBufferCreateInfo& bulkData) override;

    virtual Gfx::ORenderCommand createRenderCommand(const std::string& name) override;
    virtual Gfx::OComputeCommand createComputeCommand(const std::string& name) override;

    virtual void beginShaderCompilation(const ShaderCompilationInfo& compileInfo) override;
    virtual Gfx::OVertexShader createVertexShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OFragmentShader createFragmentShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OComputeShader createComputeShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OTaskShader createTaskShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OMeshShader createMeshShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo) override;
    virtual Gfx::PGraphicsPipeline createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo) override;
    virtual Gfx::PRayTracingPipeline createRayTracingPipeline(Gfx::RayTracingPipelineCreateInfo createInfo) override;
    virtual Gfx::PComputePipeline createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) override;
    virtual Gfx::OSampler createSampler(const SamplerCreateInfo& createInfo) override;

    virtual Gfx::ODescriptorLayout createDescriptorLayout(const std::string& name = "") override;
    virtual Gfx::OPipelineLayout createPipelineLayout(const std::string& name = "", Gfx::PPipelineLayout baseLayout = nullptr) override;

    virtual Gfx::OVertexInput createVertexInput(VertexInputStateCreateInfo createInfo) override;

    virtual Gfx::OOcclusionQuery createOcclusionQuery(const std::string& name = "") override;
    virtual Gfx::OPipelineStatisticsQuery createPipelineStatisticsQuery(const std::string& name = "") override;
    virtual Gfx::OTimestampQuery createTimestampQuery(uint64 numTimestamps, const std::string& name = "") override;

    virtual void beginDebugRegion(const std::string& name) override;
    virtual void endDebugRegion() override;

    virtual void resolveTexture(Gfx::PTexture source, Gfx::PTexture destination) override;
    virtual void copyTexture(Gfx::PTexture src, Gfx::PTexture dst) override;

    virtual void copyBuffer(Gfx::PShaderBuffer src, Gfx::PShaderBuffer dst) override;

    // Ray Tracing
    virtual Gfx::OBottomLevelAS createBottomLevelAccelerationStructure(const Gfx::BottomLevelASCreateInfo& createInfo) override;
    virtual Gfx::OTopLevelAS createTopLevelAccelerationStructure(const Gfx::TopLevelASCreateInfo& createInfo) override;
    virtual void buildBottomLevelAccelerationStructures(Array<Gfx::PBottomLevelAS> data) override;

    virtual Gfx::ORayGenShader createRayGenShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OAnyHitShader createAnyHitShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OClosestHitShader createClosestHitShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OMissShader createMissShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OIntersectionShader createIntersectionShader(const ShaderCreateInfo& createInfo) override;
    virtual Gfx::OCallableShader createCallableShader(const ShaderCreateInfo& createInfo) override;

  protected:
    Array<const char*> getRequiredExtensions();
    void initInstance(GraphicsInitializer initInfo);
    void setupDebugCallback();
    void pickPhysicalDevice();
    void createDevice(GraphicsInitializer initInfo);

    VkInstance instance;
    VkDevice handle;
    VkPhysicalDevice physicalDevice;

    Array<OQueue> queues;
    uint32 graphicsQueue;
    uint32 computeQueue;
    uint32 transferQueue;
    thread_local static PCommandPool graphicsCommands;
    thread_local static PCommandPool computeCommands;
    thread_local static PCommandPool transferCommands;
    std::mutex poolLock;
    Array<OCommandPool> pools;

    VkQueueFamilyProperties graphicsProps;

    StructChain<
        Helper<VkPhysicalDeviceProperties2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2>,
        Helper<VkPhysicalDeviceRayTracingPipelinePropertiesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR>,
        Helper<VkPhysicalDeviceAccelerationStructurePropertiesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR>>
        props;

    StructChain<Helper<VkPhysicalDeviceFeatures2, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2>,
                Helper<VkPhysicalDeviceVulkan11Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES>,
                Helper<VkPhysicalDeviceVulkan12Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES>>
        features;
    StructChain<Helper<VkPhysicalDeviceMeshShaderFeaturesEXT, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT>> meshFeatures;
    StructChain<
        Helper<VkPhysicalDeviceAccelerationStructureFeaturesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR>,
        Helper<VkPhysicalDeviceRayTracingPipelineFeaturesKHR, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR>>
        rayTracingFeatures;

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
