#include "Graphics.h"
#include "Debug.h"
#include "Allocator.h"
#include "Buffer.h"
#include "Graphics/Enums.h"
#include "PipelineCache.h"
#include "Command.h"
#include "Descriptor.h"
#include "Window.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <vulkan/vulkan_core.h>
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"

using namespace Seele;
using namespace Seele::Vulkan;

thread_local PCommandPool Seele::Vulkan::Graphics::graphicsCommands = nullptr;
thread_local PCommandPool Seele::Vulkan::Graphics::computeCommands = nullptr;
thread_local PCommandPool Seele::Vulkan::Graphics::transferCommands = nullptr;

Graphics::Graphics()
    : instance(VK_NULL_HANDLE)
    , handle(VK_NULL_HANDLE)
    , physicalDevice(VK_NULL_HANDLE)
    , callback(VK_NULL_HANDLE)
{
}

Graphics::~Graphics()
{
    vkDeviceWaitIdle(handle);
    pipelineCache = nullptr;
    allocator = nullptr;
    allocatedFramebuffers.clear();
    shaderCompiler = nullptr;
    pools.clear();
    queues.clear();
    destructionManager = nullptr;
    vkDestroyDevice(handle, nullptr);
    DestroyDebugUtilsMessengerEXT(instance, nullptr, callback);
    vkDestroyInstance(instance, nullptr);
}

void Graphics::init(GraphicsInitializer initInfo)
{
    initInstance(initInfo);
#if ENABLE_VALIDATION
    setupDebugCallback();
#endif
    pickPhysicalDevice();
    createDevice(initInfo);
    VmaAllocatorCreateInfo createInfo = {
        .physicalDevice = physicalDevice,
        .device = handle,
        .preferredLargeHeapBlockSize = 0,
        .pAllocationCallbacks = nullptr,
        .pDeviceMemoryCallbacks = nullptr,
        .pHeapSizeLimit = nullptr,
        .pVulkanFunctions = nullptr,
        .instance = instance,
        .vulkanApiVersion = VK_API_VERSION_1_3,
        .pTypeExternalMemoryHandleTypes = nullptr,
    };
    vmaCreateAllocator(&createInfo, &allocator);
    pipelineCache = new PipelineCache(this, "pipeline.cache");
    destructionManager = new DestructionManager(this);
}

Gfx::OWindow Graphics::createWindow(const WindowCreateInfo &createInfo)
{
    return new Window(this, createInfo);
}

Gfx::OViewport Graphics::createViewport(Gfx::PWindow owner, const ViewportCreateInfo &viewportInfo)
{
    return new Viewport(this, owner, viewportInfo);
}

Gfx::ORenderPass Graphics::createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies, Gfx::PViewport renderArea)
{
    return new RenderPass(this, std::move(layout), std::move(dependencies), renderArea);
}
void Graphics::beginRenderPass(Gfx::PRenderPass renderPass)
{
    PRenderPass rp = renderPass.cast<RenderPass>();
    uint32 framebufferHash = rp->getFramebufferHash();
    PFramebuffer framebuffer;
    {
        auto found = allocatedFramebuffers.find(framebufferHash);
        if (found == allocatedFramebuffers.end())
        {
            allocatedFramebuffers[framebufferHash] = new Framebuffer(this, rp, rp->getLayout());
            framebuffer = allocatedFramebuffers[framebufferHash];
        }
        else
        {
            framebuffer = std::move(found->value);
        }
    }
    getGraphicsCommands()->getCommands()->beginRenderPass(rp, framebuffer);
}

void Graphics::endRenderPass()
{
    getGraphicsCommands()->getCommands()->endRenderPass();
    getGraphicsCommands()->submitCommands();
}

void Graphics::waitDeviceIdle()
{
    vkDeviceWaitIdle(handle);    
}

void Graphics::executeCommands(const Array<Gfx::PRenderCommand>& commands)
{
    getGraphicsCommands()->getCommands()->executeCommands(commands);
}

void Graphics::executeCommands(const Array<Gfx::PComputeCommand>& commands) 
{
    getComputeCommands()->getCommands()->executeCommands(commands);
}

Gfx::OTexture2D Graphics::createTexture2D(const TextureCreateInfo &createInfo)
{
    return new Texture2D(this, createInfo);
}

Gfx::OTexture3D Graphics::createTexture3D(const TextureCreateInfo &createInfo)
{
    return new Texture3D(this, createInfo);
}

Gfx::OTextureCube Graphics::createTextureCube(const TextureCreateInfo &createInfo)
{
    return new TextureCube(this, createInfo);
}

Gfx::OUniformBuffer Graphics::createUniformBuffer(const UniformBufferCreateInfo &bulkData)
{
    return new UniformBuffer(this, bulkData);
}

Gfx::OShaderBuffer Graphics::createShaderBuffer(const ShaderBufferCreateInfo &bulkData)
{
    return new ShaderBuffer(this, bulkData);
}
Gfx::OVertexBuffer Graphics::createVertexBuffer(const VertexBufferCreateInfo &bulkData)
{
    return new VertexBuffer(this, bulkData);
}

Gfx::OIndexBuffer Graphics::createIndexBuffer(const IndexBufferCreateInfo &bulkData)
{
    return new IndexBuffer(this, bulkData);
}
Gfx::PRenderCommand Graphics::createRenderCommand(const std::string& name)
{
    return getGraphicsCommands()->createRenderCommand(name);
}

Gfx::PComputeCommand Graphics::createComputeCommand(const std::string& name) 
{
    return getComputeCommands()->createComputeCommand(name);
}

Gfx::OVertexShader Graphics::createVertexShader(const ShaderCreateInfo& createInfo)
{
    OVertexShader shader = new VertexShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OFragmentShader Graphics::createFragmentShader(const ShaderCreateInfo& createInfo)
{
    OFragmentShader shader = new FragmentShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OComputeShader Graphics::createComputeShader(const ShaderCreateInfo& createInfo) 
{
    OComputeShader shader = new ComputeShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OTaskShader Graphics::createTaskShader(const ShaderCreateInfo& createInfo)
{
    OTaskShader shader = new TaskShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OMeshShader Graphics::createMeshShader(const ShaderCreateInfo& createInfo)
{
    OMeshShader shader = new MeshShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo)
{
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo)
{
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::PComputePipeline Graphics::createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) 
{
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::OSampler Graphics::createSampler(const SamplerCreateInfo& createInfo) 
{
    OSampler sampler = new Sampler();
    VkSamplerCreateInfo vkInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .pNext = nullptr,
        .flags = createInfo.flags,
        .magFilter = cast(createInfo.magFilter),
        .minFilter = cast(createInfo.minFilter),
        .mipmapMode = cast(createInfo.mipmapMode),
        .addressModeU = cast(createInfo.addressModeU),
        .addressModeV = cast(createInfo.addressModeV),
        .addressModeW = cast(createInfo.addressModeW),
        .mipLodBias = createInfo.mipLodBias,
        .anisotropyEnable = createInfo.anisotropyEnable,
        .maxAnisotropy = createInfo.maxAnisotropy,
        .compareEnable = createInfo.compareEnable,
        .compareOp = cast(createInfo.compareOp),
        .minLod = createInfo.minLod,
        .maxLod = createInfo.maxLod,
        .borderColor = cast(createInfo.borderColor),
        .unnormalizedCoordinates = createInfo.unnormalizedCoordinates,
    };
    VK_CHECK(vkCreateSampler(handle, &vkInfo, nullptr, &sampler->sampler));
    return sampler;
}

Gfx::ODescriptorLayout Graphics::createDescriptorLayout(const std::string& name)
{
    return new DescriptorLayout(this, name);
}

Gfx::OPipelineLayout Graphics::createPipelineLayout(Gfx::PPipelineLayout baseLayout)
{
    return new PipelineLayout(this, baseLayout);
}

Gfx::OVertexInput Graphics::createVertexInput(VertexInputStateCreateInfo createInfo)
{
    return new VertexInput(createInfo);
}

void Graphics::resolveTexture(Gfx::PTexture source, Gfx::PTexture destination)
{
    PTextureBase sourceTex = source.cast<TextureBase>();
    PTextureBase destinationTex = destination.cast<TextureBase>();
    VkImageResolve resolve = {
        .srcSubresource = VkImageSubresourceLayers {
            .aspectMask = sourceTex->getAspect(),
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .srcOffset = VkOffset3D {
            .x = 0,
            .y = 0,
            .z = 0,
        },
        .dstSubresource = VkImageSubresourceLayers {
            .aspectMask = sourceTex->getAspect(),
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1,
        },
        .dstOffset = VkOffset3D {
            .x = 0,
            .y = 0,
            .z = 0,
        },
        .extent = VkExtent3D {
            .width = sourceTex->getWidth(),
            .height = sourceTex->getHeight(),
            .depth = sourceTex->getDepth(),
        },
    };
    vkCmdResolveImage(getGraphicsCommands()->getCommands()->getHandle(), sourceTex->getImage(), cast(sourceTex->getLayout()), destinationTex->getImage(), cast(destinationTex->getLayout()), 1, &resolve);
}

void Graphics::vkCmdDrawMeshTasksEXT(VkCommandBuffer handle, uint32 groupX, uint32 groupY, uint32 groupZ)
{
    cmdDrawMeshTasks(handle, groupX, groupY, groupZ);
}

void Graphics::vkSetDebugUtilsObjectNameEXT(VkDebugUtilsObjectNameInfoEXT* info)
{
    VK_CHECK(setDebugUtilsObjectName(handle, info));
}


PCommandPool Graphics::getQueueCommands(Gfx::QueueType queueType)
{
    switch (queueType)
    {
    case Gfx::QueueType::GRAPHICS:
        return getGraphicsCommands();
    case Gfx::QueueType::COMPUTE:
        return getComputeCommands();
    case Gfx::QueueType::TRANSFER:
        return getTransferCommands();
    default:
        throw new std::logic_error("invalid queue type");
    }
}
PCommandPool Graphics::getGraphicsCommands()
{
    if(graphicsCommands == nullptr)
    {
        std::unique_lock l(poolLock);
        graphicsCommands = pools.add(new CommandPool(this, queues[graphicsQueue]));
    }
    return graphicsCommands;
}
PCommandPool Graphics::getComputeCommands()
{
    if(computeCommands == nullptr)
    {
        if(graphicsQueue == computeQueue)
        {
            computeCommands = getGraphicsCommands();
        }
        else
        {
            std::unique_lock l(poolLock);
            computeCommands = pools.add(new CommandPool(this, queues[computeQueue]));
        }
    }
    return computeCommands;
}
PCommandPool Graphics::getTransferCommands()
{
    if(transferCommands == nullptr)
    {
        if(graphicsQueue == transferQueue)
        {
            transferCommands = getGraphicsCommands();
        }
        else
        {
            std::unique_lock l(poolLock);
            transferCommands = pools.add(new CommandPool(this, queues[transferQueue]));
        }
    }
    return transferCommands;
}

VmaAllocator Graphics::getAllocator()
{
    return allocator;
}

PDestructionManager Graphics::getDestructionManager()
{
    return destructionManager;
}

Array<const char *> Graphics::getRequiredExtensions()
{
    Array<const char *> extensions;

    unsigned int glfwExtensionCount = 0;
    const char **glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; i++)
    {
        extensions.add(glfwExtensions[i]);
    }
#if ENABLE_VALIDATION
    extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif // ENABLE_VALIDATION
    return extensions;
}
void Graphics::initInstance(GraphicsInitializer initInfo)
{
    glfwInit();
    assert(glfwVulkanSupported());
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = initInfo.applicationName,
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = initInfo.engineName,
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_2,
    };
    
    
    Array<const char*> extensions = getRequiredExtensions();
    for (uint32 i = 0; i < initInfo.instanceExtensions.size(); ++i)
    {
        extensions.add(initInfo.instanceExtensions[i]);
    }
#ifdef __APPLE__
    extensions.add("VK_KHR_portability_enumeration");
#endif
#if ENABLE_VALIDATION
    initInfo.layers.add("VK_LAYER_KHRONOS_validation");
#endif
    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
#if __APPLE__
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32)initInfo.layers.size(),
        .ppEnabledLayerNames = initInfo.layers.data(),
        .enabledExtensionCount = (uint32)extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VK_CHECK(vkCreateInstance(&info, nullptr, &instance));
}
void Graphics::setupDebugCallback()
{
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT| VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &debugCallback,
        .pUserData = nullptr,
    };
    VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback));
}

void Graphics::pickPhysicalDevice()
{
    uint32 physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    Array<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    uint32 deviceRating = 0;
    features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
    features.pNext = &features11;
    features11.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES;
    features11.pNext = &features12;
    features12.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES;
    features12.pNext = nullptr;
    for (auto dev : physicalDevices)
    {
        uint32 currentRating = 0;
        vkGetPhysicalDeviceProperties(dev, &props);
        vkGetPhysicalDeviceFeatures2(dev, &features);
        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            std::cout << "found dedicated gpu " << props.deviceName << std::endl;
            currentRating += 100;
        }
        else if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
        {
            std::cout << "found integrated gpu " << props.deviceName << std::endl;
            currentRating += 10;
        }
        if (currentRating > deviceRating)
        {
            deviceRating = currentRating;
            bestDevice = dev;
            std::cout << "bestDevice: " << props.deviceName << std::endl; 
        }
    }
    physicalDevice = bestDevice;

    vkGetPhysicalDeviceProperties(physicalDevice, &props);
    vkGetPhysicalDeviceFeatures2(physicalDevice, &features);
    features.features.robustBufferAccess = 0;
    if (Gfx::useMeshShading)
    {
        uint32 count = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &count, nullptr);
        Array<VkExtensionProperties> extensionProps(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &count, extensionProps.data());
        for (size_t i = 0; i < count; ++i)
        {
            if (std::strcmp(VK_EXT_MESH_SHADER_EXTENSION_NAME, extensionProps[i].extensionName) == 0)
            {
                meshShadingEnabled = true;
                break;
            }
        }
    }
}

void Graphics::createDevice(GraphicsInitializer initializer)
{
    uint32_t numQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, nullptr);
    Array<VkQueueFamilyProperties> queueProperties(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueProperties.data());

    Array<VkDeviceQueueCreateInfo> queueInfos;
    struct QueueCreateInfo
    {
        int32 familyIndex = -1;
        int32 queueIndex = -1;
    };
    QueueCreateInfo graphicsQueueInfo;
    QueueCreateInfo transferQueueInfo;
    QueueCreateInfo computeQueueInfo;
    uint32 numPriorities = 0;
    auto checkFamilyProperty = [](VkQueueFamilyProperties currProps, uint32 checkBit){
        return (currProps.queueFlags & checkBit) == checkBit;
    };
    auto updateQueueInfo = [&queueProperties](uint32 familyIndex, QueueCreateInfo& info, uint32& numQueues) {
        if(info.familyIndex == -1) {
            if(queueProperties[familyIndex].queueCount == numQueues)
            {
                return;
            }
            info.familyIndex = familyIndex;
            info.queueIndex = numQueues++;
        }
    };
    for (uint32 familyIndex = 0; familyIndex < queueProperties.size(); ++familyIndex)
    {
        uint32 numQueuesForFamily = 0;
        VkQueueFamilyProperties currProps = queueProperties[familyIndex];

        if (checkFamilyProperty(currProps, VK_QUEUE_GRAPHICS_BIT))
        {
            updateQueueInfo(familyIndex, graphicsQueueInfo, numQueuesForFamily);
        }
        if(Gfx::useAsyncCompute)
        {
            if(checkFamilyProperty(currProps, VK_QUEUE_COMPUTE_BIT))
            {
                updateQueueInfo(familyIndex, computeQueueInfo, numQueuesForFamily);
            }
        }
        if ((currProps.queueFlags ^ VK_QUEUE_TRANSFER_BIT) == 0)
        {
            updateQueueInfo(familyIndex, transferQueueInfo, numQueuesForFamily);
        }
        
        if (numQueuesForFamily > 0)
        {
            VkDeviceQueueCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = familyIndex,
                .queueCount = numQueuesForFamily,
            };
            numPriorities += numQueuesForFamily;
            queueInfos.add(info);
        }
    }
    Array<float> queuePriorities;
    queuePriorities.resize(numPriorities);
    float *currentPriority = queuePriorities.data();
    for (uint32 index = 0; index < queueInfos.size(); ++index)
    {
        VkDeviceQueueCreateInfo &currQueue = queueInfos[index];
        currQueue.pQueuePriorities = currentPriority;

        for (int32_t queueIndex = 0; queueIndex < (int32_t)currQueue.queueCount; ++queueIndex)
        {
            *currentPriority++ = 1.0f;
        }
    }
    VkPhysicalDeviceMeshShaderFeaturesEXT enabledMeshShaderFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        .pNext = nullptr,
        .taskShader = VK_TRUE,
        .meshShader = VK_TRUE,
    };
    if (supportMeshShading())
    {
        features12.pNext = &enabledMeshShaderFeatures;
        initializer.deviceExtensions.add(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }
#ifdef __APPLE__
    initializer.deviceExtensions.add("VK_KHR_portability_subset");
#endif
    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features11,
        .queueCreateInfoCount = (uint32)queueInfos.size(),
        .pQueueCreateInfos = queueInfos.data(),
        .enabledExtensionCount = (uint32)initializer.deviceExtensions.size(),
        .ppEnabledExtensionNames = initializer.deviceExtensions.data(),
        .pEnabledFeatures = &features.features,
    };

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &handle));
    //std::cout << "Vulkan handle: " << handle << std::endl;

    cmdDrawMeshTasks = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(handle, "vkCmdDrawMeshTasksEXT");

    graphicsQueue = 0;
    computeQueue = 0;
    transferQueue = 0;
    queues.add(new Queue(this, graphicsQueueInfo.familyIndex, graphicsQueueInfo.queueIndex));
    if(computeQueueInfo.familyIndex != -1)
    {
        computeQueue = queues.size();
        queues.add(new Queue(this, computeQueueInfo.familyIndex, computeQueueInfo.queueIndex));
    }
    if(transferQueueInfo.familyIndex != -1)
    {
        transferQueue = queues.size();
        queues.add(new Queue(this, transferQueueInfo.familyIndex, transferQueueInfo.queueIndex));
    }
    
    // if (Gfx::useAsyncCompute && asyncComputeInfo.familyIndex != -1)
    // {
    //     if (asyncComputeInfo.familyIndex == graphicsQueueInfo.familyIndex)
    //     {
    //         // Same family as graphics, but different queue
    //         computeQueue = new Queue(this, asyncComputeInfo.familyIndex, 1);
    //     }
    //     else
    //     {
    //         // Different family
    //         computeQueue = new Queue(this, asyncComputeInfo.familyIndex, 0);
    //     }
    // }
    // else
    // {
    //     computeQueue = new Queue(this, computeQueueInfo.familyIndex, 0);
    // }
    // transferQueue = new Queue(this, transferQueueInfo.familyIndex, 0);
    // if (dedicatedTransferQueueInfo.familyIndex != -1)
    // {
    //     dedicatedTransferQueue = new Queue(this, dedicatedTransferQueueInfo.familyIndex, 0);
    // }
    queueMapping.graphicsFamily = queues[graphicsQueue]->getFamilyIndex();
    queueMapping.computeFamily = queues[computeQueue]->getFamilyIndex();
    queueMapping.transferFamily = queues[transferQueue]->getFamilyIndex();
    setDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");

}
