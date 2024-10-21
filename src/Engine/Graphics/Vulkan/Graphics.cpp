#include "Graphics.h"
#include "Allocator.h"
#include "Buffer.h"
#include "Command.h"
#include "Debug.h"
#include "Descriptor.h"
#include "Framebuffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/StaticMeshVertexData.h"
#include "Graphics/slang-compile.h"
#include "PipelineCache.h"
#include "Query.h"
#include "RayTracing.h"
#include "RenderPass.h"
#include "Shader.h"
#include "Window.h"
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

PFN_vkCmdDrawMeshTasksEXT cmdDrawMeshTasks;
PFN_vkCmdDrawMeshTasksIndirectEXT cmdDrawMeshTasksIndirect;
PFN_vkSetDebugUtilsObjectNameEXT setDebugUtilsObjectName;
PFN_vkQueueBeginDebugUtilsLabelEXT queueBeginDebugUtilsLabelEXT;
PFN_vkQueueEndDebugUtilsLabelEXT queueEndDebugUtilsLabelEXT;
PFN_vkCreateAccelerationStructureKHR createAccelerationStructure;
PFN_vkCmdBuildAccelerationStructuresKHR cmdBuildAccelerationStructures;
PFN_vkGetAccelerationStructureBuildSizesKHR getAccelerationStructureBuildSize;
PFN_vkCreateRayTracingPipelinesKHR createRayTracingPipelines;
PFN_vkGetRayTracingShaderGroupHandlesKHR getRayTracingShaderGroupHandles;
PFN_vkCmdTraceRaysKHR cmdTraceRays;
PFN_vkGetAccelerationStructureDeviceAddressKHR getAccelerationStructureDeviceAddress;

void vkCmdDrawMeshTasksEXT(VkCommandBuffer command, uint32 groupX, uint32 groupY, uint32 groupZ) {
    cmdDrawMeshTasks(command, groupX, groupY, groupZ);
}

void vkCmdDrawMeshTasksIndirectEXT(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
                                   uint32_t stride) {
    cmdDrawMeshTasksIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

VkResult vkSetDebugUtilsObjectNameEXT(VkDevice device, const VkDebugUtilsObjectNameInfoEXT* pNameInfo) {
    if (setDebugUtilsObjectName != nullptr) {
        return setDebugUtilsObjectName(device, pNameInfo);
    }
    return VK_SUCCESS;
}

void vkQueueBeginDebugUtilsLabelEXT(VkQueue queue, const VkDebugUtilsLabelEXT* pLabelInfo) {
    if (queueBeginDebugUtilsLabelEXT != nullptr) {
        queueBeginDebugUtilsLabelEXT(queue, pLabelInfo);
    }
}

void vkQueueEndDebugUtilsLabelEXT(VkQueue queue) {
    if (queueEndDebugUtilsLabelEXT != nullptr) {
        queueEndDebugUtilsLabelEXT(queue);
    }
}

VkResult vkCreateAccelerationStructureKHR(VkDevice device, const VkAccelerationStructureCreateInfoKHR* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator, VkAccelerationStructureKHR* pAccelerationStructure) {
    return createAccelerationStructure(device, pCreateInfo, pAllocator, pAccelerationStructure);
}

void vkCmdBuildAccelerationStructuresKHR(VkCommandBuffer commandBuffer, uint32_t infoCount,
                                         const VkAccelerationStructureBuildGeometryInfoKHR* pInfos,
                                         const VkAccelerationStructureBuildRangeInfoKHR* const* ppBuildRangeInfos) {
    cmdBuildAccelerationStructures(commandBuffer, infoCount, pInfos, ppBuildRangeInfos);
}
void vkGetAccelerationStructureBuildSizesKHR(VkDevice device, VkAccelerationStructureBuildTypeKHR buildType,
                                             const VkAccelerationStructureBuildGeometryInfoKHR* pBuildInfo,
                                             const uint32_t* pMaxPrimitiveCounts, VkAccelerationStructureBuildSizesInfoKHR* pSizeInfo) {
    getAccelerationStructureBuildSize(device, buildType, pBuildInfo, pMaxPrimitiveCounts, pSizeInfo);
}

VkResult vkCreateRayTracingPipelinesKHR(VkDevice device, VkDeferredOperationKHR deferredOperation, VkPipelineCache pipelineCache,
                                        uint32_t createInfoCount, const VkRayTracingPipelineCreateInfoKHR* pCreateInfos,
                                        const VkAllocationCallbacks* pAllocator, VkPipeline* pPipelines) {
    return createRayTracingPipelines(device, deferredOperation, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult vkGetRayTracingShaderGroupHandlesKHR(VkDevice device, VkPipeline pipeline, uint32_t firstGroup, uint32_t groupCount,
                                              size_t dataSize, void* pData) {
    return getRayTracingShaderGroupHandles(device, pipeline, firstGroup, groupCount, dataSize, pData);
}

void vkCmdTraceRaysKHR(VkCommandBuffer commandBuffer, const VkStridedDeviceAddressRegionKHR* pRaygenShaderBindingTable,
                       const VkStridedDeviceAddressRegionKHR* pMissShaderBindingTable,
                       const VkStridedDeviceAddressRegionKHR* pHitShaderBindingTable,
                       const VkStridedDeviceAddressRegionKHR* pCallableShaderBindingTable, uint32_t width, uint32_t height,
                       uint32_t depth) {
    cmdTraceRays(commandBuffer, pRaygenShaderBindingTable, pMissShaderBindingTable, pHitShaderBindingTable, pCallableShaderBindingTable,
                 width, height, depth);
}

VkDeviceAddress vkGetAccelerationStructureDeviceAddressKHR(VkDevice device, const VkAccelerationStructureDeviceAddressInfoKHR* pInfo) {
    return getAccelerationStructureDeviceAddress(device, pInfo);
}

Graphics::Graphics() : instance(VK_NULL_HANDLE), handle(VK_NULL_HANDLE), physicalDevice(VK_NULL_HANDLE), callback(VK_NULL_HANDLE) {}

Graphics::~Graphics() {
    pipelineCache = nullptr;
    allocatedFramebuffers.clear();
    shaderCompiler = nullptr;
    pools.clear();
    queues.clear();
    destructionManager = nullptr;
    allocator = nullptr;
    vkDestroyDevice(handle, nullptr);
    DestroyDebugUtilsMessengerEXT(instance, nullptr, callback);
    vkDestroyInstance(instance, nullptr);
}

void Graphics::init(GraphicsInitializer initInfo) {
    initInstance(initInfo);
    pickPhysicalDevice();
    createDevice(initInfo);
    VmaAllocatorCreateInfo createInfo = {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT | VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
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

Gfx::OWindow Graphics::createWindow(const WindowCreateInfo& createInfo) { return new Window(this, createInfo); }

Gfx::OViewport Graphics::createViewport(Gfx::PWindow owner, const ViewportCreateInfo& viewportInfo) {
    return new Viewport(owner, viewportInfo);
}

Gfx::ORenderPass Graphics::createRenderPass(Gfx::RenderTargetLayout layout, Array<Gfx::SubPassDependency> dependencies,
                                            Gfx::PViewport renderArea, std::string name) {
    return new RenderPass(this, std::move(layout), std::move(dependencies), renderArea, name);
}
void Graphics::beginRenderPass(Gfx::PRenderPass renderPass) {
    PRenderPass rp = renderPass.cast<RenderPass>();
    uint32 framebufferHash = rp->getFramebufferHash();
    PFramebuffer framebuffer;
    {
        auto found = allocatedFramebuffers.find(framebufferHash);
        if (found == allocatedFramebuffers.end()) {
            allocatedFramebuffers[framebufferHash] = new Framebuffer(this, rp, rp->getLayout());
            framebuffer = allocatedFramebuffers[framebufferHash];
        } else {
            framebuffer = std::move(found->value);
        }
    }
    getGraphicsCommands()->getCommands()->beginRenderPass(rp, framebuffer);
}

void Graphics::endRenderPass() { getGraphicsCommands()->getCommands()->endRenderPass(); }

void Graphics::waitDeviceIdle() {
    getGraphicsCommands()->submitCommands();
    vkDeviceWaitIdle(handle);
    getGraphicsCommands()->refreshCommands();
}

void Graphics::executeCommands(Array<Gfx::ORenderCommand> commands) {
    getGraphicsCommands()->getCommands()->executeCommands(std::move(commands));
}

void Graphics::executeCommands(Gfx::OComputeCommand commands) {
    Array<Gfx::OComputeCommand> commandArray;
    commandArray.add(std::move(commands));
    getComputeCommands()->getCommands()->executeCommands(std::move(commandArray));
}

void Graphics::executeCommands(Array<Gfx::OComputeCommand> commands) {
    getComputeCommands()->getCommands()->executeCommands(std::move(commands));
}

Gfx::OTexture2D Graphics::createTexture2D(const TextureCreateInfo& createInfo) { return new Texture2D(this, createInfo); }

Gfx::OTexture3D Graphics::createTexture3D(const TextureCreateInfo& createInfo) { return new Texture3D(this, createInfo); }

Gfx::OTextureCube Graphics::createTextureCube(const TextureCreateInfo& createInfo) { return new TextureCube(this, createInfo); }

Gfx::OUniformBuffer Graphics::createUniformBuffer(const UniformBufferCreateInfo& bulkData) { return new UniformBuffer(this, bulkData); }

Gfx::OShaderBuffer Graphics::createShaderBuffer(const ShaderBufferCreateInfo& bulkData) { return new ShaderBuffer(this, bulkData); }

Gfx::OVertexBuffer Graphics::createVertexBuffer(const VertexBufferCreateInfo& bulkData) { return new VertexBuffer(this, bulkData); }

Gfx::OIndexBuffer Graphics::createIndexBuffer(const IndexBufferCreateInfo& bulkData) { return new IndexBuffer(this, bulkData); }

Gfx::ORenderCommand Graphics::createRenderCommand(const std::string& name) { return getGraphicsCommands()->createRenderCommand(name); }

Gfx::OComputeCommand Graphics::createComputeCommand(const std::string& name) { return getComputeCommands()->createComputeCommand(name); }

void Graphics::beginShaderCompilation(const ShaderCompilationInfo& createInfo) {
    beginCompilation(createInfo, SLANG_SPIRV, createInfo.rootSignature);
}

Gfx::OVertexShader Graphics::createVertexShader(const ShaderCreateInfo& createInfo) {
    OVertexShader shader = new VertexShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OFragmentShader Graphics::createFragmentShader(const ShaderCreateInfo& createInfo) {
    OFragmentShader shader = new FragmentShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OComputeShader Graphics::createComputeShader(const ShaderCreateInfo& createInfo) {
    OComputeShader shader = new ComputeShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OTaskShader Graphics::createTaskShader(const ShaderCreateInfo& createInfo) {
    OTaskShader shader = new TaskShader(this);
    shader->create(createInfo);
    return shader;
}
Gfx::OMeshShader Graphics::createMeshShader(const ShaderCreateInfo& createInfo) {
    OMeshShader shader = new MeshShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(Gfx::LegacyPipelineCreateInfo createInfo) {
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::PGraphicsPipeline Graphics::createGraphicsPipeline(Gfx::MeshPipelineCreateInfo createInfo) {
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::PRayTracingPipeline Graphics::createRayTracingPipeline(Gfx::RayTracingPipelineCreateInfo createInfo) {
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::PComputePipeline Graphics::createComputePipeline(Gfx::ComputePipelineCreateInfo createInfo) {
    return pipelineCache->createPipeline(std::move(createInfo));
}

Gfx::OSampler Graphics::createSampler(const SamplerCreateInfo& createInfo) {
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
    return new Sampler(this, vkInfo);
}

Gfx::OComputeShader Graphics::createComputeShaderFromBinary(std::string_view binaryName) {
    OComputeShader shader = new ComputeShader(this);
    shader->create(binaryName);
    return shader;
}

Gfx::ODescriptorLayout Graphics::createDescriptorLayout(const std::string& name) { return new DescriptorLayout(this, name); }

Gfx::OPipelineLayout Graphics::createPipelineLayout(const std::string& name, Gfx::PPipelineLayout baseLayout) {
    return new PipelineLayout(this, name, baseLayout);
}

Gfx::OVertexInput Graphics::createVertexInput(VertexInputStateCreateInfo createInfo) { return new VertexInput(createInfo); }

Gfx::OOcclusionQuery Graphics::createOcclusionQuery(const std::string& name) { return new OcclusionQuery(this, name); }

Gfx::OPipelineStatisticsQuery Graphics::createPipelineStatisticsQuery(const std::string& name) {
    return new PipelineStatisticsQuery(this, name);
}

Gfx::OTimestampQuery Graphics::createTimestampQuery(uint64 numTimestamps, const std::string& name) {
    return new TimestampQuery(this, name);
}

void Graphics::beginDebugRegion(const std::string& name) {
    VkDebugUtilsLabelEXT label = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT,
        .pNext = nullptr,
        .pLabelName = name.c_str(),
    };
    vkQueueBeginDebugUtilsLabelEXT(queues[graphicsQueue]->getHandle(), &label);
}

void Graphics::endDebugRegion() { vkQueueEndDebugUtilsLabelEXT(queues[graphicsQueue]->getHandle()); }

void Graphics::resolveTexture(Gfx::PTexture source, Gfx::PTexture destination) {
    PTextureBase sourceTex = source.cast<TextureBase>();
    PTextureBase destinationTex = destination.cast<TextureBase>();
    VkImageResolve resolve = {
        .srcSubresource =
            VkImageSubresourceLayers{
                .aspectMask = sourceTex->getAspect(),
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .srcOffset =
            VkOffset3D{
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .dstSubresource =
            VkImageSubresourceLayers{
                .aspectMask = sourceTex->getAspect(),
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .dstOffset =
            VkOffset3D{
                .x = 0,
                .y = 0,
                .z = 0,
            },
        .extent =
            VkExtent3D{
                .width = sourceTex->getWidth(),
                .height = sourceTex->getHeight(),
                .depth = sourceTex->getDepth(),
            },
    };
    vkCmdResolveImage(getGraphicsCommands()->getCommands()->getHandle(), sourceTex->getImage(), cast(sourceTex->getLayout()),
                      destinationTex->getImage(), cast(destinationTex->getLayout()), 1, &resolve);
}

void Graphics::copyTexture(Gfx::PTexture source, Gfx::PTexture destination) {
    PTextureBase src = source.cast<TextureBase>();
    PTextureBase dst = destination.cast<TextureBase>();
    Gfx::SeImageLayout srcLayout = src->getLayout();
    Gfx::SeImageLayout dstLayout = dst->getLayout();
    src->changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, Gfx::SE_ACCESS_MEMORY_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
                      Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);
    dst->changeLayout(Gfx::SE_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, Gfx::SE_ACCESS_MEMORY_WRITE_BIT | Gfx::SE_ACCESS_MEMORY_READ_BIT,
                      Gfx::SE_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT);

    VkImageBlit blit = {
        .srcSubresource =
            {
                .aspectMask = src->getAspect(),
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .srcOffsets =
            {
                {0, 0, 0},
                {(int32)src->getWidth(), (int32)src->getHeight(), (int32)src->getDepth()},
            },
        .dstSubresource =
            {
                .aspectMask = dst->getAspect(),
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1,
            },
        .dstOffsets =
            {
                {0, 0, 0},
                {(int32)dst->getWidth(), (int32)dst->getHeight(), (int32)dst->getDepth()},
            },
    };
    PCommand command = getGraphicsCommands()->getCommands();
    vkCmdBlitImage(command->getHandle(), src->getImage(), VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, dst->getImage(),
                   VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &blit,
                   src->getAspect() & VK_IMAGE_ASPECT_DEPTH_BIT ? VK_FILTER_NEAREST : VK_FILTER_LINEAR);

    src->changeLayout(srcLayout, Gfx::SE_ACCESS_TRANSFER_READ_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                      Gfx::SE_ACCESS_MEMORY_READ_BIT | Gfx::SE_ACCESS_MEMORY_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);

    dst->changeLayout(dstLayout, Gfx::SE_ACCESS_TRANSFER_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TRANSFER_BIT,
                      Gfx::SE_ACCESS_MEMORY_READ_BIT | Gfx::SE_ACCESS_MEMORY_WRITE_BIT, Gfx::SE_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
}

void Graphics::copyBuffer(Gfx::PShaderBuffer srcBuffer, Gfx::PShaderBuffer dstBuffer) {
    PShaderBuffer src = srcBuffer.cast<ShaderBuffer>();
    PShaderBuffer dst = dstBuffer.cast<ShaderBuffer>();

    VkBufferCopy region = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = src->getSize(),
    };
    vkCmdCopyBuffer(getGraphicsCommands()->getCommands()->getHandle(), src->getHandle(), dst->getHandle(), 1, &region);
}

Gfx::OBottomLevelAS Graphics::createBottomLevelAccelerationStructure(const Gfx::BottomLevelASCreateInfo& createInfo) {
    return new BottomLevelAS(this, createInfo);
}

Gfx::OTopLevelAS Graphics::createTopLevelAccelerationStructure(const Gfx::TopLevelASCreateInfo& createInfo) {
    return new TopLevelAS(this, createInfo);
}

void Graphics::buildBottomLevelAccelerationStructures(Array<Gfx::PBottomLevelAS> data) {
    Gfx::PShaderBuffer verticesBuffer = StaticMeshVertexData::getInstance()->getPositionBuffer();
    Gfx::PIndexBuffer indexBuffer = StaticMeshVertexData::getInstance()->getIndexBuffer();

    // Setup vertices and indices for a single triangle

    // Create buffers for the bottom level geometry
    // For the sake of simplicity we won't stage the vertex data to the GPU memory

    // Note that the buffer usage flags for buffers consumed by the bottom level acceleration structure require special flags
    const VkBufferUsageFlags buffer_usage_flags = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                  VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    VkBufferCreateInfo transformBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(VkTransformMatrixKHR) * data.size(),
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    VmaAllocationCreateInfo transformAllocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };

    Array<VkTransformMatrixKHR> matrices;
    for (const auto gfxBlas : data) {
        const auto blas = gfxBlas.cast<BottomLevelAS>();
        matrices.add(blas->getTransform());
    }
    OBufferAllocation transformBuffer =
        new BufferAllocation(this, "TransformBuffer", transformBufferInfo, transformAllocInfo, Gfx::QueueType::GRAPHICS);
    transformBuffer->updateContents(0, sizeof(VkTransformMatrixKHR) * matrices.size(), matrices.data());
    transformBuffer->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                                     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_TRANSFER_READ_BIT,
                                     VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR | VK_PIPELINE_STAGE_TRANSFER_BIT);
    verticesBuffer->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                    VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);
    indexBuffer->pipelineBarrier(VK_ACCESS_TRANSFER_WRITE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_ACCESS_SHADER_READ_BIT,
                                 VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR);

    Array<VkAccelerationStructureGeometryKHR> geometries(data.size());
    Array<VkAccelerationStructureBuildGeometryInfoKHR> buildGeometries(data.size());
    Array<VkAccelerationStructureBuildSizesInfoKHR> buildSizes(data.size());
    Array<OBufferAllocation> scratchBuffers(data.size());
    Array<VkAccelerationStructureBuildRangeInfoKHR> buildRanges(data.size());
    Array<VkAccelerationStructureBuildRangeInfoKHR*> buildRangePointers(data.size());
    for (uint32 i = 0; i < data.size(); ++i) {
        auto blas = data[i].cast<BottomLevelAS>();
        VkDeviceOrHostAddressConstKHR vertexDataAddress = {
            .deviceAddress = verticesBuffer.cast<ShaderBuffer>()->getDeviceAddress() + blas->getVertexOffset(),
        };
        VkDeviceOrHostAddressConstKHR indexDataAddress = {
            .deviceAddress = indexBuffer.cast<IndexBuffer>()->getDeviceAddress() + blas->getIndexOffset(),
        };
        VkDeviceOrHostAddressConstKHR transformDataAddress = {
            .deviceAddress = transformBuffer->deviceAddress + i * sizeof(VkTransformMatrixKHR),
        };
        geometries[i] = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
            .pNext = nullptr,
            .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
            .geometry =
                {
                    .triangles =
                        {
                            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
                            .pNext = nullptr,
                            .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
                            .vertexData = vertexDataAddress,
                            .vertexStride = sizeof(Vector4),
                            .maxVertex = static_cast<uint32_t>(blas->getVertexCount()),
                            .indexType = VK_INDEX_TYPE_UINT32,
                            .indexData = indexDataAddress,
                            .transformData = transformDataAddress,
                        },
                },
            .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
        };
        buildGeometries[i] = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
            .pNext = nullptr,
            .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
            .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
            .geometryCount = 1,
            .pGeometries = &geometries[i],
        };

        buildSizes[i] = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
            .pNext = nullptr,
        };
        const uint32 primitiveCount = blas->getPrimitiveCount();
        vkGetAccelerationStructureBuildSizesKHR(handle, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildGeometries[i],
                                                &primitiveCount, &buildSizes[i]);

        VkBufferCreateInfo bufferInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = buildSizes[i].accelerationStructureSize,
            .usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        };
        VmaAllocationCreateInfo bufferAllocInfo = {
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        blas->buffer = new BufferAllocation(this, "BLAS", bufferInfo, bufferAllocInfo, Gfx::QueueType::GRAPHICS);

        VkAccelerationStructureCreateInfoKHR blasInfo = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
            .pNext = nullptr,
            .createFlags = 0,
            .buffer = blas->buffer->buffer,
            .offset = 0,
            .size = buildSizes[i].accelerationStructureSize,
            .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        };
        VK_CHECK(vkCreateAccelerationStructureKHR(handle, &blasInfo, nullptr, &blas->handle));

        VkBufferCreateInfo scratchInfo = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = buildSizes[i].buildScratchSize,
            .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        };
        VmaAllocationCreateInfo scratchAllocInfo = {
            .usage = VMA_MEMORY_USAGE_AUTO,
        };
        scratchBuffers[i] = new BufferAllocation(this, "ScratchBuffer", scratchInfo, scratchAllocInfo, Gfx::QueueType::GRAPHICS,
                                                 accelerationProperties.minAccelerationStructureScratchOffsetAlignment);

        buildGeometries[i].dstAccelerationStructure = blas->handle;
        buildGeometries[i].scratchData.deviceAddress = scratchBuffers[i]->deviceAddress;

        buildRanges[i] = VkAccelerationStructureBuildRangeInfoKHR{
            .primitiveCount = primitiveCount,
            .primitiveOffset = 0,
            .firstVertex = 0,
            .transformOffset = 0,
        };
        buildRangePointers[i] = &buildRanges[i];
    }

    PCommand cmd = graphicsCommands->getCommands();
    vkCmdBuildAccelerationStructuresKHR(cmd->getHandle(), buildGeometries.size(), buildGeometries.data(), buildRangePointers.data());
    cmd->bindResource(PBufferAllocation(transformBuffer));
    destructionManager->queueResourceForDestruction(std::move(transformBuffer));
    for (auto& scratchAlloc : scratchBuffers) {
        cmd->bindResource(PBufferAllocation(scratchAlloc));
        destructionManager->queueResourceForDestruction(std::move(scratchAlloc));
    }
}

Gfx::ORayGenShader Graphics::createRayGenShader(const ShaderCreateInfo& createInfo) {
    ORayGenShader shader = new RayGenShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::OAnyHitShader Graphics::createAnyHitShader(const ShaderCreateInfo& createInfo) {
    OAnyHitShader shader = new AnyHitShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::OClosestHitShader Graphics::createClosestHitShader(const ShaderCreateInfo& createInfo) {
    OClosestHitShader shader = new ClosestHitShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::OMissShader Graphics::createMissShader(const ShaderCreateInfo& createInfo) {
    OMissShader shader = new MissShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::OIntersectionShader Graphics::createIntersectionShader(const ShaderCreateInfo& createInfo) {
    OIntersectionShader shader = new IntersectionShader(this);
    shader->create(createInfo);
    return shader;
}

Gfx::OCallableShader Graphics::createCallableShader(const ShaderCreateInfo& createInfo) {
    OCallableShader shader = new CallableShader(this);
    shader->create(createInfo);
    return shader;
}

PCommandPool Graphics::getQueueCommands(Gfx::QueueType queueType) {
    switch (queueType) {
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
PCommandPool Graphics::getGraphicsCommands() {
    if (graphicsCommands == nullptr) {
        std::unique_lock l(poolLock);
        graphicsCommands = pools.add(new CommandPool(this, queues[graphicsQueue]));
    }
    return graphicsCommands;
}
PCommandPool Graphics::getComputeCommands() {
    if (computeCommands == nullptr) {
        if (graphicsQueue == computeQueue) {
            computeCommands = getGraphicsCommands();
        } else {
            std::unique_lock l(poolLock);
            computeCommands = pools.add(new CommandPool(this, queues[computeQueue]));
        }
    }
    return computeCommands;
}
PCommandPool Graphics::getTransferCommands() {
    if (transferCommands == nullptr) {
        if (graphicsQueue == transferQueue) {
            transferCommands = getGraphicsCommands();
        } else {
            std::unique_lock l(poolLock);
            transferCommands = pools.add(new CommandPool(this, queues[transferQueue]));
        }
    }
    return transferCommands;
}

VmaAllocator Graphics::getAllocator() const { return allocator; }

PDestructionManager Graphics::getDestructionManager() { return destructionManager; }

Array<const char*> Graphics::getRequiredExtensions() {
    Array<const char*> extensions;

    unsigned int glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    for (unsigned int i = 0; i < glfwExtensionCount; i++) {
        extensions.add(glfwExtensions[i]);
    }
#ifdef ENABLE_VALIDATION
    extensions.add(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
    return extensions;
}

void Graphics::initInstance(GraphicsInitializer initInfo) {
    glfwInit();
    assert(glfwVulkanSupported());
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = initInfo.applicationName,
        .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
        .pEngineName = initInfo.engineName,
        .engineVersion = VK_MAKE_VERSION(0, 0, 1),
        .apiVersion = VK_API_VERSION_1_3,
    };

    Array<const char*> extensions = getRequiredExtensions();
    for (uint32 i = 0; i < initInfo.instanceExtensions.size(); ++i) {
        extensions.add(initInfo.instanceExtensions[i]);
    }
#ifdef __APPLE__
    extensions.add("VK_KHR_portability_enumeration");
#endif
    Array<const char*> layers = initInfo.layers;
    // layers.add("VK_LAYER_KHRONOS_validation");
    VkInstanceCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
#if __APPLE__
        .flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR,
#endif
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32)layers.size(),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = (uint32)extensions.size(),
        .ppEnabledExtensionNames = extensions.data(),
    };
    VK_CHECK(vkCreateInstance(&info, nullptr, &instance));
}

void Graphics::setupDebugCallback() {
    VkDebugUtilsMessengerCreateInfoEXT createInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .pNext = nullptr,
        .flags = 0,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                       VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = &debugCallback,
        .pUserData = nullptr,
    };
    VK_CHECK(CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &callback));
}

void Graphics::pickPhysicalDevice() {
    uint32 physicalDeviceCount;
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);
    Array<VkPhysicalDevice> physicalDevices(physicalDeviceCount);
    vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices.data());
    VkPhysicalDevice bestDevice = VK_NULL_HANDLE;
    uint32 deviceRating = 0;
    props = VkPhysicalDeviceProperties2{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2,
        .pNext = &accelerationProperties,
    };
    accelerationProperties = VkPhysicalDeviceAccelerationStructurePropertiesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_PROPERTIES_KHR,
        .pNext = &rayTracingProperties,
    };
    rayTracingProperties = VkPhysicalDeviceRayTracingPipelinePropertiesKHR{
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR,
        .pNext = nullptr,
    };
    for (auto dev : physicalDevices) {
        uint32 currentRating = 0;
        vkGetPhysicalDeviceProperties2(dev, &props);
        if (props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            std::cout << "found dedicated gpu " << props.properties.deviceName << std::endl;
            currentRating += 100;
        } else if (props.properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
            std::cout << "found integrated gpu " << props.properties.deviceName << std::endl;
            currentRating += 10;
        }
        if (currentRating > deviceRating) {
            deviceRating = currentRating;
            bestDevice = dev;
            std::cout << "bestDevice: " << props.properties.deviceName << std::endl;
        }
    }
    physicalDevice = bestDevice;

    vkGetPhysicalDeviceProperties2(physicalDevice, &props);
    rayTracingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR,
        .pNext = nullptr,
        .rayTracingPipeline = true,
        .rayTraversalPrimitiveCulling = true,
    };
    accelerationFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR,
        .pNext = &rayTracingFeatures,
        .accelerationStructure = true,
    };
    meshShaderFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT,
        .pNext = nullptr,
        .taskShader = true,
        .meshShader = true,
        .meshShaderQueries = true,
    };
    features12 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES,
        .pNext = &meshShaderFeatures,
        .storageBuffer8BitAccess = true,
        .uniformAndStorageBuffer8BitAccess = true,
        .shaderBufferInt64Atomics = true,
        .shaderFloat16 = true,
        .shaderInt8 = true,
        .shaderUniformBufferArrayNonUniformIndexing = true,
        .shaderSampledImageArrayNonUniformIndexing = true,
        .shaderStorageBufferArrayNonUniformIndexing = true,
        .descriptorBindingUniformBufferUpdateAfterBind = true,
        .descriptorBindingSampledImageUpdateAfterBind = true,
        .descriptorBindingStorageBufferUpdateAfterBind = true,
        .descriptorBindingPartiallyBound = true,
        .runtimeDescriptorArray = true,
        .bufferDeviceAddress = true,
    };
    features11 = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_1_FEATURES,
        .pNext = &features12,
        .storageBuffer16BitAccess = true,
        .uniformAndStorageBuffer16BitAccess = true,
    };
    features = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
        .pNext = &features11,
        .features =
            {
                //.robustBufferAccess = true,
                .geometryShader = true,
                .fillModeNonSolid = true,
                .wideLines = true,
                .samplerAnisotropy = true,
                .pipelineStatisticsQuery = true,
                .fragmentStoresAndAtomics = true,
                .shaderInt64 = true,
                .shaderInt16 = true,
                .inheritedQueries = true,
            },
    };
    if (Gfx::useMeshShading) {
        uint32 count = 0;
        vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &count, nullptr);
        Array<VkExtensionProperties> extensionProps(count);
        vkEnumerateDeviceExtensionProperties(physicalDevice, NULL, &count, extensionProps.data());
        for (size_t i = 0; i < count; ++i) {
            if (std::strcmp(VK_EXT_MESH_SHADER_EXTENSION_NAME, extensionProps[i].extensionName) == 0) {
                meshShadingEnabled = true;
                break;
            }
        }
    }
}

void Graphics::createDevice(GraphicsInitializer initializer) {
    uint32_t numQueueFamilies = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, nullptr);
    Array<VkQueueFamilyProperties> queueProperties(numQueueFamilies);
    vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &numQueueFamilies, queueProperties.data());

    Array<VkDeviceQueueCreateInfo> queueInfos;
    struct QueueCreateInfo {
        int32 familyIndex = -1;
        int32 queueIndex = -1;
    };
    QueueCreateInfo graphicsQueueInfo;
    QueueCreateInfo transferQueueInfo;
    QueueCreateInfo computeQueueInfo;
    uint32 numPriorities = 0;
    auto checkFamilyProperty = [](VkQueueFamilyProperties currProps, uint32 checkBit) {
        return (currProps.queueFlags & checkBit) == checkBit;
    };
    auto updateQueueInfo = [&queueProperties](uint32 familyIndex, QueueCreateInfo& info, uint32& numQueues) {
        if (info.familyIndex == -1) {
            if (queueProperties[familyIndex].queueCount == numQueues) {
                return;
            }
            info.familyIndex = familyIndex;
            info.queueIndex = numQueues++;
        }
    };
    for (uint32 familyIndex = 0; familyIndex < queueProperties.size(); ++familyIndex) {
        uint32 numQueuesForFamily = 0;
        VkQueueFamilyProperties currProps = queueProperties[familyIndex];

        if (checkFamilyProperty(currProps, VK_QUEUE_GRAPHICS_BIT)) {
            updateQueueInfo(familyIndex, graphicsQueueInfo, numQueuesForFamily);
        }
        if (Gfx::useAsyncCompute) {
            if (checkFamilyProperty(currProps, VK_QUEUE_COMPUTE_BIT)) {
                updateQueueInfo(familyIndex, computeQueueInfo, numQueuesForFamily);
            }
        }
        if ((currProps.queueFlags ^ VK_QUEUE_TRANSFER_BIT) == 0) {
            updateQueueInfo(familyIndex, transferQueueInfo, numQueuesForFamily);
        }

        if (numQueuesForFamily > 0) {
            VkDeviceQueueCreateInfo info = {
                .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
                .pNext = nullptr,
                .flags = 0,
                .queueFamilyIndex = familyIndex,
                .queueCount = numQueuesForFamily,
                .pQueuePriorities = nullptr,
            };
            numPriorities += numQueuesForFamily;
            queueInfos.add(info);
        }
    }
    Array<float> queuePriorities;
    queuePriorities.resize(numPriorities);
    float* currentPriority = queuePriorities.data();
    for (uint32 index = 0; index < queueInfos.size(); ++index) {
        VkDeviceQueueCreateInfo& currQueue = queueInfos[index];
        currQueue.pQueuePriorities = currentPriority;

        for (int32_t queueIndex = 0; queueIndex < (int32_t)currQueue.queueCount; ++queueIndex) {
            *currentPriority++ = 1.0f;
        }
    }
    if (supportMeshShading()) {
        initializer.deviceExtensions.add(VK_EXT_MESH_SHADER_EXTENSION_NAME);
    }
#ifdef __APPLE__
    initializer.deviceExtensions.add("VK_KHR_portability_subset");
#endif
    // initializer.deviceExtensions.add(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
    // initializer.deviceExtensions.add(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
    // initializer.deviceExtensions.add(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &features,
        .queueCreateInfoCount = (uint32)queueInfos.size(),
        .pQueueCreateInfos = queueInfos.data(),
        .enabledExtensionCount = (uint32)initializer.deviceExtensions.size(),
        .ppEnabledExtensionNames = initializer.deviceExtensions.data(),
        .pEnabledFeatures = nullptr,
    };
    VK_CHECK(vkCreateDevice(physicalDevice, &deviceInfo, nullptr, &handle));
    graphicsQueue = 0;
    computeQueue = 0;
    transferQueue = 0;
    queues.add(new Queue(this, graphicsQueueInfo.familyIndex, graphicsQueueInfo.queueIndex));
    if (computeQueueInfo.familyIndex != -1) {
        computeQueue = queues.size();
        queues.add(new Queue(this, computeQueueInfo.familyIndex, computeQueueInfo.queueIndex));
    }
    if (transferQueueInfo.familyIndex != -1) {
        transferQueue = queues.size();
        queues.add(new Queue(this, transferQueueInfo.familyIndex, transferQueueInfo.queueIndex));
    }

    queueMapping.graphicsFamily = queues[graphicsQueue]->getFamilyIndex();
    queueMapping.computeFamily = queues[computeQueue]->getFamilyIndex();
    queueMapping.transferFamily = queues[transferQueue]->getFamilyIndex();

    graphicsProps = queueProperties[queueMapping.graphicsFamily];

    cmdDrawMeshTasks = (PFN_vkCmdDrawMeshTasksEXT)vkGetDeviceProcAddr(handle, "vkCmdDrawMeshTasksEXT");
    cmdDrawMeshTasksIndirect = (PFN_vkCmdDrawMeshTasksIndirectEXT)vkGetDeviceProcAddr(handle, "vkCmdDrawMeshTasksIndirectEXT");
    setDebugUtilsObjectName = (PFN_vkSetDebugUtilsObjectNameEXT)vkGetInstanceProcAddr(instance, "vkSetDebugUtilsObjectNameEXT");
    queueBeginDebugUtilsLabelEXT = (PFN_vkQueueBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueBeginDebugUtilsLabelEXT");
    queueEndDebugUtilsLabelEXT = (PFN_vkQueueEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(instance, "vkQueueEndDebugUtilsLabelEXT");

    createAccelerationStructure = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(handle, "vkCreateAccelerationStructureKHR");
    cmdBuildAccelerationStructures =
        (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(handle, "vkCmdBuildAccelerationStructuresKHR");
    getAccelerationStructureBuildSize =
        (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(handle, "vkGetAccelerationStructureBuildSizesKHR");
    createRayTracingPipelines = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(handle, "vkCreateRayTracingPipelinesKHR");
    getRayTracingShaderGroupHandles =
        (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(handle, "vkGetRayTracingShaderGroupHandlesKHR");
    cmdTraceRays = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(handle, "vkCmdTraceRaysKHR");
    getAccelerationStructureDeviceAddress =
        (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(handle, "vkGetAccelerationStructureDeviceAddressKHR");
}
