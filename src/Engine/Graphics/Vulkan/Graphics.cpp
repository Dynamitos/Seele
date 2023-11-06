#include "Containers/Array.h"
#include "Graphics.h"
#include "Allocator.h"
#include "Buffer.h"
#include "Graphics/Enums.h"
#include "PipelineCache.h"
#include "CommandBuffer.h"
#include "Initializer.h"
#include "RenderTarget.h"
#include "RenderPass.h"
#include "Framebuffer.h"
#include "Shader.h"
#include <GLFW/glfw3.h>
#include <cstring>
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Vulkan;

thread_local OCommandBufferManager Seele::Vulkan::Graphics::graphicsCommands = nullptr;
thread_local OCommandBufferManager Seele::Vulkan::Graphics::computeCommands = nullptr;
thread_local OCommandBufferManager Seele::Vulkan::Graphics::transferCommands = nullptr;
thread_local OCommandBufferManager Seele::Vulkan::Graphics::dedicatedTransferCommands = nullptr;

Graphics::Graphics()
    : instance(VK_NULL_HANDLE)
    , handle(VK_NULL_HANDLE)
    , physicalDevice(VK_NULL_HANDLE)
    , callback(VK_NULL_HANDLE)
{
}

Graphics::~Graphics()
{
    viewports.clear();
    vkDestroyDevice(handle, nullptr);
    DestroyDebugReportCallbackEXT(instance, nullptr, callback);
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
    allocator = new Allocator(this);
    stagingManager = new StagingManager(this, allocator);
    pipelineCache = new PipelineCache(this, "pipeline.cache");
}

Gfx::OWindow Graphics::createWindow(const WindowCreateInfo &createInfo)
{
    OWindow result = new Window(this, createInfo);
    return result;
}

Gfx::OViewport Graphics::createViewport(Gfx::PWindow owner, const ViewportCreateInfo &viewportInfo)
{
    OViewport result = new Viewport(this, owner, viewportInfo);
    std::scoped_lock lock(viewportLock);
    viewports.add(result);
    return result;
}
Gfx::ORenderPass Graphics::createRenderPass(Gfx::ORenderTargetLayout layout, Gfx::PViewport renderArea)
{
    ORenderPass result = new RenderPass(this, std::move(layout), renderArea);
    return result;
}
void Graphics::beginRenderPass(Gfx::PRenderPass renderPass)
{
    PRenderPass rp = renderPass.cast<RenderPass>();
    uint32 framebufferHash = rp->getFramebufferHash();
    PFramebuffer framebuffer;
    {
        std::scoped_lock lock(allocatedFrameBufferLock);
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
    std::scoped_lock lock(renderPassLock);
    activeRenderPass = rp;
    activeFramebuffer = framebuffer;
}

void Graphics::endRenderPass()
{
    getGraphicsCommands()->getCommands()->endRenderPass();
    getGraphicsCommands()->submitCommands();
    std::scoped_lock lock(renderPassLock);
    activeRenderPass = nullptr;
    activeFramebuffer = nullptr;
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
    OTexture2D result = new Texture2D(this, createInfo);
    return result;
}

Gfx::OTexture3D Graphics::createTexture3D(const TextureCreateInfo &createInfo)
{
    OTexture3D result = new Texture3D(this, createInfo);
    return result;
}

Gfx::OTextureCube Graphics::createTextureCube(const TextureCreateInfo &createInfo)
{
    OTextureCube result = new TextureCube(this, createInfo);
    return result;
}

Gfx::OUniformBuffer Graphics::createUniformBuffer(const UniformBufferCreateInfo &bulkData)
{
    OUniformBuffer uniformBuffer = new UniformBuffer(this, bulkData);
    return uniformBuffer;
}

Gfx::OShaderBuffer Graphics::createShaderBuffer(const ShaderBufferCreateInfo &bulkData)
{
    OShaderBuffer shaderBuffer = new ShaderBuffer(this, bulkData);
    return shaderBuffer;
}
Gfx::OVertexBuffer Graphics::createVertexBuffer(const VertexBufferCreateInfo &bulkData)
{
    OVertexBuffer vertexBuffer = new VertexBuffer(this, bulkData);
    return vertexBuffer;
}

Gfx::OIndexBuffer Graphics::createIndexBuffer(const IndexBufferCreateInfo &bulkData)
{
    OIndexBuffer indexBuffer = new IndexBuffer(this, bulkData);
    return indexBuffer;
}
Gfx::PRenderCommand Graphics::createRenderCommand(const std::string& name)
{
    PRenderCommand cmdBuffer = getGraphicsCommands()->createRenderCommand(activeRenderPass, activeFramebuffer, name);
    return cmdBuffer;
}

Gfx::PComputeCommand Graphics::createComputeCommand(const std::string& name) 
{
    PComputeCommand cmdBuffer = getComputeCommands()->createComputeCommand(name);
    return cmdBuffer;
}

Gfx::OVertexDeclaration Graphics::createVertexDeclaration(const Array<Gfx::VertexElement>& element) 
{
    OVertexDeclaration declaration = new VertexDeclaration(element);
    return declaration;
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

Gfx::OGraphicsPipeline Graphics::createGraphicsPipeline(const Gfx::LegacyPipelineCreateInfo& createInfo)
{
    OGraphicsPipeline pipeline = pipelineCache->createPipeline(createInfo);
    return pipeline;
}
Gfx::OGraphicsPipeline Graphics::createGraphicsPipeline(const Gfx::MeshPipelineCreateInfo& createInfo)
{
    OGraphicsPipeline pipeline = pipelineCache->createPipeline(createInfo);
    return pipeline;
}

Gfx::OComputePipeline Graphics::createComputePipeline(const Gfx::ComputePipelineCreateInfo& createInfo) 
{
    OComputePipeline pipeline = pipelineCache->createPipeline(createInfo);
    return pipeline;
}

Gfx::OSamplerState Graphics::createSamplerState(const SamplerCreateInfo& createInfo) 
{
    OSamplerState sampler = new SamplerState();
    VkSamplerCreateInfo vkInfo = 
        init::SamplerCreateInfo();
    vkInfo.addressModeU = cast(createInfo.addressModeU);
    vkInfo.addressModeV = cast(createInfo.addressModeV);
    vkInfo.addressModeW = cast(createInfo.addressModeW);
    vkInfo.anisotropyEnable = createInfo.anisotropyEnable;
    vkInfo.borderColor = cast(createInfo.borderColor);
    vkInfo.compareEnable = createInfo.compareEnable;
    vkInfo.compareOp = cast(createInfo.compareOp);
    vkInfo.flags = createInfo.flags;
    vkInfo.magFilter = cast(createInfo.magFilter);
    vkInfo.maxAnisotropy = createInfo.maxAnisotropy;
    vkInfo.maxLod = createInfo.maxLod;
    vkInfo.minFilter = cast(createInfo.minFilter);
    vkInfo.minLod = createInfo.minLod;
    vkInfo.mipLodBias = createInfo.mipLodBias;
    vkInfo.mipmapMode = cast(createInfo.mipmapMode);
    vkInfo.unnormalizedCoordinates = createInfo.unnormalizedCoordinates;
    VK_CHECK(vkCreateSampler(handle, &vkInfo, nullptr, &sampler->sampler));
    return sampler;
}
Gfx::ODescriptorLayout Graphics::createDescriptorLayout(const std::string& name)
{
    ODescriptorLayout layout = new DescriptorLayout(this, name);
    return layout;
}
Gfx::OPipelineLayout Graphics::createPipelineLayout(Gfx::PPipelineLayout baseLayout)
{
    OPipelineLayout layout = new PipelineLayout(this, baseLayout);
    return layout;
}

void Graphics::copyTexture(Gfx::PTexture srcTexture, Gfx::PTexture dstTexture) 
{
    Texture2D* src = (Texture2D*)srcTexture->getTexture2D();
    Texture2D* dst = (Texture2D*)dstTexture->getTexture2D();
    TextureHandle* srcHandle = (TextureHandle*)src->getNativeHandle();
    TextureHandle* dstHandle = (TextureHandle*)dst->getNativeHandle();
    Gfx::SeImageLayout srcLayout = srcHandle->getLayout();
    Gfx::SeImageLayout dstLayout = dstHandle->getLayout();
    Gfx::QueueType dstOwner = dstHandle->currentOwner;
    src->changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);
    dst->changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    dstTexture->transferOwnership(srcHandle->currentOwner);
    PCmdBuffer cmdBuffer = getQueueCommands(srcHandle->currentOwner)->getCommands();
    if(srcHandle->getAspect() != dstHandle->getAspect())
    {
        /*VkMemoryRequirements imageRequirements;
        vkGetImageMemoryRequirements(handle, srcHandle->getImage(), &imageRequirements);
        PShaderBuffer tempBuffer = createShaderBuffer();
        VkBufferImageCopy bufferImageCopy;
        bufferImageCopy.bufferOffset = 0;
        bufferImageCopy.bufferRowLength = srcTexture->getSizeX();
        bufferImageCopy.bufferImageHeight = srcTexture->getSizeY();
        bufferImageCopy.imageExtent.width = srcTexture->getSizeX();
        bufferImageCopy.imageExtent.height = srcTexture->getSizeY();
        bufferImageCopy.imageExtent.depth = 1;
        bufferImageCopy.imageOffset.x = 0;
        bufferImageCopy.imageOffset.y = 0;
        bufferImageCopy.imageOffset.z = 0;
        bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        bufferImageCopy.imageSubresource.baseArrayLayer = 0;
        bufferImageCopy.imageSubresource.layerCount = 1;
        bufferImageCopy.imageSubresource.mipLevel = 0;

        vkCmdCopyImageToBuffer(cmdBuffer->getHandle(), srcHandle->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, tempBufferAllocation->getHandle(), 1, &bufferImageCopy);

        bufferImageCopy.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        
        vkCmdCopyBufferToImage(cmdBuffer->getHandle(), tempBufferAllocation->getHandle(), dstHandle->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
        delete tempBufferAllocation;*/
        throw new std::logic_error("Not yet implemented!");
    }
    else if (src->getSizeX() != dst->getSizeX()
          || src->getSizeY() != dst->getSizeY())
    {
        throw new std::logic_error("Not yet implemented!");
    }
    else
    {
        VkImageCopy copy;
        std::memset(&copy, 0, sizeof(VkImageCopy));
        copy.extent.width = srcTexture->getSizeX();
        copy.extent.height = srcTexture->getSizeY();
        copy.extent.depth = 1;
        copy.srcSubresource.aspectMask = srcHandle->getAspect();
        copy.srcSubresource.layerCount = 1;
        copy.dstSubresource.aspectMask = dstHandle->getAspect();
        copy.dstSubresource.layerCount = 1;
        vkCmdCopyImage(cmdBuffer->getHandle(),
                srcHandle->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                dstHandle->getImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1, &copy);
        src->changeLayout(srcLayout);
        dst->changeLayout(dstLayout);
        dstTexture->transferOwnership(dstOwner);
    }
}

void Graphics::vkCmdDrawMeshTasksEXT(VkCommandBuffer handle, uint32 groupX, uint32 groupY, uint32 groupZ)
{
    cmdDrawMeshTasks(handle, groupX, groupY, groupZ);
}
PCommandBufferManager Graphics::getQueueCommands(Gfx::QueueType queueType)
{
    switch (queueType)
    {
    case Gfx::QueueType::GRAPHICS:
        return getGraphicsCommands();
    case Gfx::QueueType::COMPUTE:
        return getComputeCommands();
    case Gfx::QueueType::TRANSFER:
        return getTransferCommands();
    case Gfx::QueueType::DEDICATED_TRANSFER:
        return getDedicatedTransferCommands();
    default:
        throw new std::logic_error("invalid queue type");
    }
}
PCommandBufferManager Graphics::getGraphicsCommands()
{
    if(graphicsCommands == nullptr)
    {
        graphicsCommands = new CommandBufferManager(this, graphicsQueue);
    }
    return graphicsCommands;
}
PCommandBufferManager Graphics::getComputeCommands()
{
    if(computeCommands == nullptr)
    {
        computeCommands = new CommandBufferManager(this, computeQueue);
    }
    return computeCommands;
}
PCommandBufferManager Graphics::getTransferCommands()
{
    if(transferCommands == nullptr)
    {
        transferCommands = new CommandBufferManager(this, transferQueue);
    }
    return transferCommands;
}
PCommandBufferManager Graphics::getDedicatedTransferCommands()
{
    if(dedicatedTransferCommands == nullptr)
    {
        dedicatedTransferCommands = new CommandBufferManager(this, dedicatedTransferQueue != nullptr ? dedicatedTransferQueue : transferQueue);
    }
    return dedicatedTransferCommands;
}
PAllocator Graphics::getAllocator()
{
    return allocator;
}

PStagingManager Graphics::getStagingManager()
{
    return stagingManager;
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
    extensions.add(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif // ENABLE_VALIDATION
    return extensions;
}
void Graphics::initInstance(GraphicsInitializer initInfo)
{
    glfwInit();
    VkApplicationInfo appInfo = {};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = initInfo.applicationName;
    appInfo.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.pEngineName = initInfo.engineName;
    appInfo.engineVersion = VK_MAKE_VERSION(0, 0, 1);
    appInfo.apiVersion = VK_API_VERSION_1_2;

    VkInstanceCreateInfo info = {};
    info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    info.pApplicationInfo = &appInfo;
    Array<const char *> extensions = getRequiredExtensions();
    for (uint32 i = 0; i < initInfo.instanceExtensions.size(); ++i)
    {
        extensions.add(initInfo.instanceExtensions[i]);
    }
    info.enabledExtensionCount = (uint32)extensions.size();
    info.ppEnabledExtensionNames = extensions.data();
#if ENABLE_VALIDATION
    info.enabledLayerCount = (uint32)initInfo.layers.size();
    info.ppEnabledLayerNames = initInfo.layers.data();
#else
    info.enabledLayerCount = 0;
#endif
    VK_CHECK(vkCreateInstance(&info, nullptr, &instance));
}
void Graphics::setupDebugCallback()
{
    VkDebugReportCallbackCreateInfoEXT createInfo =
        init::DebugReportCallbackCreateInfo(
            VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT | VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT);

    VK_CHECK(CreateDebugReportCallbackEXT(instance, &createInfo, nullptr, &callback));
}

void Graphics::pickPhysicalDevice()
{
    uint32 physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    Array<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    uint32 deviceRating = 0;
    for (auto dev : physicalDevices)
    {
        uint32 currentRating = 0;
        vkGetPhysicalDeviceProperties(dev, &props);
        vkGetPhysicalDeviceFeatures(dev, &features);
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
    vkGetPhysicalDeviceFeatures(physicalDevice, &features);
    uint32 count = 0;
    vkEnumerateDeviceExtensionProperties(physicalDevice, "VK_EXT_mesh_shader", &count, nullptr);
    Array<VkExtensionProperties> extensionProps(count);
    vkEnumerateDeviceExtensionProperties(physicalDevice, "VK_EXT_mesh_shader", &count, extensionProps.data());
    for(size_t i = 0; i < count; ++i)
    {
        if(std::strcmp("VK_EXT_mesh_shader", extensionProps[i].extensionName) == 0)
        {
            Gfx::useMeshShading = true;
            break;
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
    QueueCreateInfo dedicatedTransferQueueInfo;
    QueueCreateInfo computeQueueInfo;
    QueueCreateInfo asyncComputeInfo;
    uint32 numPriorities = 0;
    auto checkFamilyProperty = [](VkQueueFamilyProperties currProps, uint32 checkBit){
        return (currProps.queueFlags & checkBit) == checkBit;
    };
    for (uint32 familyIndex = 0; familyIndex < queueProperties.size(); ++familyIndex)
    {
        uint32 numQueuesForFamily = 0;
        VkQueueFamilyProperties currProps = queueProperties[familyIndex];

        if (checkFamilyProperty(currProps, VK_QUEUE_GRAPHICS_BIT))
        {
            if (graphicsQueueInfo.familyIndex == -1)
            {
                graphicsQueueInfo.familyIndex = familyIndex;
                numQueuesForFamily++;
            }
        }
        if (currProps.queueFlags & VK_QUEUE_COMPUTE_BIT)
        {
            if (computeQueueInfo.familyIndex == -1)
            {
                computeQueueInfo.familyIndex = familyIndex;
            }
            if (Gfx::useAsyncCompute && numQueueFamilies > 1)
            {
                if (asyncComputeInfo.familyIndex == -1)
                {
                    if (familyIndex == (uint32)graphicsQueueInfo.familyIndex)
                    {
                        if (currProps.queueCount > 1)
                        {
                            asyncComputeInfo.familyIndex = familyIndex;
                            numQueuesForFamily++;
                        }
                    }
                    else
                    {
                        asyncComputeInfo.familyIndex = familyIndex;
                    }
                }
            }
        }
        if (currProps.queueFlags & VK_QUEUE_TRANSFER_BIT)
        {
            if (transferQueueInfo.familyIndex == -1)
            {
                transferQueueInfo.familyIndex = familyIndex;
                numQueuesForFamily++;
            }
            if ((currProps.queueFlags ^ VK_QUEUE_TRANSFER_BIT) == 0)
            {
                dedicatedTransferQueueInfo.familyIndex = familyIndex;
                numQueuesForFamily++;
            }
        }
        if (numQueuesForFamily > 0)
        {
            VkDeviceQueueCreateInfo info =
                init::DeviceQueueCreateInfo(familyIndex, numQueuesForFamily);
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
    VkDeviceCreateInfo deviceInfo = init::DeviceCreateInfo(
        queueInfos.data(),
        (uint32)queueInfos.size(),
        nullptr);

    VkPhysicalDeviceDescriptorIndexingFeatures descriptorIndexing = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DESCRIPTOR_INDEXING_FEATURES,
        .shaderSampledImageArrayNonUniformIndexing = VK_TRUE,
        .descriptorBindingPartiallyBound = VK_TRUE,
        .descriptorBindingVariableDescriptorCount = VK_TRUE,
        .runtimeDescriptorArray = VK_TRUE,
    };
    deviceInfo.pNext = &descriptorIndexing;
#if ENABLE_VALIDATION
    VkDeviceDiagnosticsConfigCreateInfoNV crashDiagInfo;
    crashDiagInfo.sType = VK_STRUCTURE_TYPE_DEVICE_DIAGNOSTICS_CONFIG_CREATE_INFO_NV;
    crashDiagInfo.pNext = nullptr;
    crashDiagInfo.flags = 
        VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_RESOURCE_TRACKING_BIT_NV |
        VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_AUTOMATIC_CHECKPOINTS_BIT_NV |
        VK_DEVICE_DIAGNOSTICS_CONFIG_ENABLE_SHADER_DEBUG_INFO_BIT_NV;
    descriptorIndexing.pNext = &crashDiagInfo;
#endif
    VkPhysicalDeviceMeshShaderFeaturesEXT enabledMeshShaderFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        .taskShader = VK_TRUE,
        .meshShader = VK_TRUE,
    };
    if (Gfx::useMeshShading)
    {
        descriptorIndexing.pNext = &enabledMeshShaderFeatures;
        initializer.deviceExtensions.add("VK_EXT_mesh_shader");
    }
    deviceInfo.enabledExtensionCount = (uint32)initializer.deviceExtensions.size();
    deviceInfo.ppEnabledExtensionNames = initializer.deviceExtensions.data();
    deviceInfo.enabledLayerCount = (uint32_t)initializer.layers.size();
    deviceInfo.ppEnabledLayerNames = initializer.layers.data();
    deviceInfo.pEnabledFeatures = &features;

    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &handle));
    std::cout << "Vulkan handle: " << handle << std::endl;

    cmdDrawMeshTasks = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(handle, "vkCmdDrawMeshTasksEXT");

    graphicsQueue = new Queue(this, Gfx::QueueType::GRAPHICS, graphicsQueueInfo.familyIndex, 0);
    if (Gfx::useAsyncCompute && asyncComputeInfo.familyIndex != -1)
    {
        if (asyncComputeInfo.familyIndex == graphicsQueueInfo.familyIndex)
        {
            // Same family as graphics, but different queue
            computeQueue = new Queue(this, Gfx::QueueType::COMPUTE, asyncComputeInfo.familyIndex, 1);
        }
        else
        {
            // Different family
            computeQueue = new Queue(this, Gfx::QueueType::COMPUTE, asyncComputeInfo.familyIndex, 0);
        }
    }
    else
    {
        computeQueue = new Queue(this, Gfx::QueueType::COMPUTE, computeQueueInfo.familyIndex, 0);
    }
    transferQueue = new Queue(this, Gfx::QueueType::TRANSFER, transferQueueInfo.familyIndex, 0);
    if (dedicatedTransferQueueInfo.familyIndex != -1)
    {
        dedicatedTransferQueue = new Queue(this, Gfx::QueueType::DEDICATED_TRANSFER, dedicatedTransferQueueInfo.familyIndex, 0);
    }
    queueMapping.graphicsFamily = graphicsQueue->getFamilyIndex();
    queueMapping.computeFamily = computeQueue->getFamilyIndex();
    queueMapping.transferFamily = transferQueue->getFamilyIndex();
    queueMapping.dedicatedTransferFamily = dedicatedTransferQueue != nullptr ? dedicatedTransferQueue->getFamilyIndex() : transferQueue->getFamilyIndex();
}
