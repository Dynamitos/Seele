#include "RayTracing.h"
#include "Buffer.h"
#include "Command.h"
#include "Enums.h"
#include "Graphics/Buffer.h"
#include "Graphics/Enums.h"
#include "Graphics/Graphics.h"
#include "Graphics/Initializer.h"
#include "Graphics/Mesh.h"
#include "Graphics/VertexData.h"
#include <vulkan/vulkan_core.h>

using namespace Seele::Vulkan;

BottomLevelAS::BottomLevelAS(PGraphics graphics, const Gfx::BottomLevelASCreateInfo& createInfo)
    : graphics(graphics), material(createInfo.mesh->referencedMaterial->getHandle()) {
    VertexData* vertexData = createInfo.mesh->vertexData;
    MeshData meshData = vertexData->getMeshData(createInfo.mesh->id);
    Gfx::PShaderBuffer positionBuffer = vertexData->getPositionBuffer();
    Gfx::PIndexBuffer indexBuffer = vertexData->getIndexBuffer();
    VkTransformMatrixKHR matrix = {
        createInfo.mesh->transform[0][0], createInfo.mesh->transform[1][0], createInfo.mesh->transform[2][0],
        createInfo.mesh->transform[3][0], createInfo.mesh->transform[0][1], createInfo.mesh->transform[1][1],
        createInfo.mesh->transform[2][1], createInfo.mesh->transform[3][1], createInfo.mesh->transform[0][2],
        createInfo.mesh->transform[1][2], createInfo.mesh->transform[2][2], createInfo.mesh->transform[3][2],
    };
    VkBufferCreateInfo transformBufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = sizeof(VkTransformMatrixKHR),
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    };
    VmaAllocationCreateInfo transformAllocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    OBufferAllocation transformBuffer =
        new BufferAllocation(graphics, "TransformBuffer", transformBufferInfo, transformAllocInfo, Gfx::QueueType::GRAPHICS);
    transformBuffer->updateContents(0, sizeof(VkTransformMatrixKHR), &matrix);

    VkDeviceOrHostAddressConstKHR vertexDataAddress = {
        .deviceAddress =
            positionBuffer.cast<ShaderBuffer>()->getDeviceAddress() + vertexData->getMeshOffset(createInfo.mesh->id) * sizeof(Vector4),
    };
    VkDeviceOrHostAddressConstKHR indexDataAddress = {
        .deviceAddress = indexBuffer.cast<IndexBuffer>()->getDeviceAddress() + meshData.firstIndex * sizeof(uint32),
    };
    VkDeviceOrHostAddressConstKHR transformDataAddress = {
        .deviceAddress = transformBuffer->deviceAddress,
    };
    VkAccelerationStructureGeometryKHR geometry = {
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
                        .vertexStride = sizeof(Vector),
                        .maxVertex = static_cast<uint32_t>(createInfo.mesh->vertexCount),
                        .indexType = VK_INDEX_TYPE_UINT32,
                        .indexData = indexDataAddress,
                        .transformData = transformDataAddress,
                    },
            },
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };
    VkAccelerationStructureBuildGeometryInfoKHR structureBuildGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext = nullptr,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .geometryCount = 1,
        .pGeometries = &geometry,
    };

    const uint32 primitiveCount = meshData.numIndices / 3;

    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
        .pNext = nullptr,
    };
    vkGetAccelerationStructureBuildSizesKHR(graphics->getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &structureBuildGeometry,
                                            &primitiveCount, &buildSizesInfo);

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = buildSizesInfo.accelerationStructureSize,
        .usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };
    VmaAllocationCreateInfo bufferAllocInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    buffer = new BufferAllocation(graphics, "BLAS", bufferInfo, bufferAllocInfo, Gfx::QueueType::GRAPHICS);

    VkAccelerationStructureCreateInfoKHR blasInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = 0,
        .buffer = buffer->buffer,
        .offset = 0,
        .size = buildSizesInfo.accelerationStructureSize,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
    };
    VK_CHECK(vkCreateAccelerationStructureKHR(graphics->getDevice(), &blasInfo, nullptr, &handle));

    VkBufferCreateInfo scratchInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .size = buildSizesInfo.buildScratchSize,
        .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
    };
    VmaAllocationCreateInfo scratchAllocInfo = {
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    OBufferAllocation scratchAlloc =
        new BufferAllocation(graphics, "ScratchBuffer", scratchInfo, scratchAllocInfo, Gfx::QueueType::GRAPHICS,
                             graphics->getAccelerationProperties().minAccelerationStructureScratchOffsetAlignment);

    structureBuildGeometry.dstAccelerationStructure = handle;
    structureBuildGeometry.scratchData.deviceAddress = scratchAlloc->deviceAddress;

    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
        .primitiveCount = primitiveCount,
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0,
    };
    Array<VkAccelerationStructureBuildRangeInfoKHR*> ranges = {&buildRangeInfo};

    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdBuildAccelerationStructuresKHR(cmd->getHandle(), 1, &structureBuildGeometry, ranges.data());
    scratchAlloc->bind();
    buffer->bind();
    transformBuffer->bind();
    cmd->bindResource(PBufferAllocation(transformBuffer));
    cmd->bindResource(PBufferAllocation(buffer));
    cmd->bindResource(PBufferAllocation(scratchAlloc));

    graphics->getDestructionManager()->queueResourceForDestruction(std::move(transformBuffer));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(scratchAlloc));
    // todo: compact
}

BottomLevelAS::~BottomLevelAS() { graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffer)); }

TopLevelAS::TopLevelAS(PGraphics graphics, const Gfx::TopLevelASCreateInfo& createInfo) {
    Array<VkAccelerationStructureInstanceKHR> instances(createInfo.instances.size());
    for (uint32 i = 0; i < instances.size(); ++i) {
        auto blas = createInfo.bottomLevelStructures[i].cast<BottomLevelAS>();

        instances[i] = VkAccelerationStructureInstanceKHR{
            .transform =
                VkTransformMatrixKHR{
                    createInfo.instances[i].transformMatrix[0][0],
                    createInfo.instances[i].transformMatrix[1][0],
                    createInfo.instances[i].transformMatrix[2][0],
                    createInfo.instances[i].transformMatrix[3][0],
                    createInfo.instances[i].transformMatrix[0][1],
                    createInfo.instances[i].transformMatrix[1][1],
                    createInfo.instances[i].transformMatrix[2][1],
                    createInfo.instances[i].transformMatrix[3][1],
                    createInfo.instances[i].transformMatrix[0][2],
                    createInfo.instances[i].transformMatrix[1][2],
                    createInfo.instances[i].transformMatrix[2][2],
                    createInfo.instances[i].transformMatrix[3][2],
                },
            .instanceCustomIndex = i,
            .mask = 0xff,
            .instanceShaderBindingTableRecordOffset = i,
            .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
            .accelerationStructureReference = blas->getDeviceAddress(),
        };
    }

    instanceAllocation = new BufferAllocation(
        graphics, "ASInstances",
        VkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = sizeof(VkAccelerationStructureInstanceKHR) * instances.size(),
            .usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
        },
        VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO,
        },
        Gfx::QueueType::GRAPHICS);

    VkDeviceOrHostAddressConstKHR instanceDeviceAddress = {
        .deviceAddress = instanceAllocation->deviceAddress,
    };

    VkAccelerationStructureGeometryKHR geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry = {.instances =
                         {
                             .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                             .pNext = nullptr,
                             .arrayOfPointers = VK_FALSE,
                             .data = instanceDeviceAddress,
                         }},
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };
    VkAccelerationStructureBuildGeometryInfoKHR structureBuildGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext = nullptr,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
        .geometryCount = 1,
        .pGeometries = &geometry,
    };

    const uint32 primitiveCount = instances.size();

    VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR,
        .pNext = nullptr,
    };
    vkGetAccelerationStructureBuildSizesKHR(graphics->getDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &structureBuildGeometry,
                                            &primitiveCount, &buildSizesInfo);

    buffer = new BufferAllocation(
        graphics, "TLAS",
        VkBufferCreateInfo{
            .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
            .pNext = nullptr,
            .flags = 0,
            .size = buildSizesInfo.accelerationStructureSize,
            .usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,

        },
        VmaAllocationCreateInfo{
            .usage = VMA_MEMORY_USAGE_AUTO,
        },
        Gfx::QueueType::GRAPHICS);

    VkAccelerationStructureCreateInfoKHR accelerationInfo = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .buffer = buffer->buffer,
        .size = buildSizesInfo.accelerationStructureSize,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
    };
    VK_CHECK(vkCreateAccelerationStructureKHR(graphics->getDevice(), &accelerationInfo, nullptr, &handle));

    OBufferAllocation scratchBuffer =
        new BufferAllocation(graphics, "ScratchBuffer",
                             VkBufferCreateInfo{
                                 .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                 .pNext = nullptr,
                                 .flags = 0,
                                 .size = buildSizesInfo.buildScratchSize,
                                 .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,

                             },
                             VmaAllocationCreateInfo{
                                 .usage = VMA_MEMORY_USAGE_AUTO,
                             },
                             Gfx::QueueType::GRAPHICS);

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext = nullptr,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_BUILD_BIT_KHR,
        .mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
        .dstAccelerationStructure = handle,
        .geometryCount = 1,
        .pGeometries = &geometry,
        .scratchData =
            {
                .deviceAddress = scratchBuffer->deviceAddress,
            },
    };
    VkAccelerationStructureBuildRangeInfoKHR buildRange = {
        .primitiveCount = uint32(instances.size()),
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0,
    };
    VkAccelerationStructureBuildRangeInfoKHR* buildRangeInfos[] = {&buildRange};

    auto cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdBuildAccelerationStructuresKHR(cmd->getHandle(), 1, &buildGeometry, buildRangeInfos);
    scratchBuffer->bind();

    graphics->getDestructionManager()->queueResourceForDestruction(std::move(scratchBuffer));
}

TopLevelAS::~TopLevelAS() {}

RayTracingPipeline::RayTracingPipeline(PGraphics graphics, VkPipeline handle, OBufferAllocation rayGen, uint64 rayGenStride,
                                       OBufferAllocation hit, uint64 hitStride, OBufferAllocation miss, uint64 missStride,
                                       Gfx::PPipelineLayout layout)
    : Gfx::RayTracingPipeline(layout), graphics(graphics), pipeline(handle), rayGen(std::move(rayGen)), rayGenStride(rayGenStride),
      hit(std::move(hit)), hitStride(hitStride), miss(std::move(miss)), missStride(missStride) {}

RayTracingPipeline::~RayTracingPipeline() {}

void RayTracingPipeline::bind(VkCommandBuffer handle) { vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline); }
