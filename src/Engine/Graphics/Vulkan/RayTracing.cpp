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

BottomLevelAS::BottomLevelAS(PGraphics graphics, const Gfx::BottomLevelASCreateInfo& createInfo) : graphics(graphics) {
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
                 VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
    };
    VmaAllocationCreateInfo transformAllocInfo = {
        .flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO,
    };
    OBufferAllocation transformBuffer =
        new BufferAllocation(graphics, "TransformBuffer", transformBufferInfo, transformAllocInfo, Gfx::QueueType::GRAPHICS);
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
        .geometry = {.triangles =
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
    buffer =
        new BufferAllocation(graphics, "BLAS", bufferInfo, bufferAllocInfo, Gfx::QueueType::GRAPHICS);

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
    cmd->bindResource(PBufferAllocation(transformBuffer));
    cmd->bindResource(PBufferAllocation(buffer));
    cmd->bindResource(PBufferAllocation(scratchAlloc));

    graphics->getDestructionManager()->queueResourceForDestruction(std::move(transformBuffer));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(scratchAlloc));
    //todo: compact
}

BottomLevelAS::~BottomLevelAS() { graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffer)); }

TopLevelAS::TopLevelAS(PGraphics graphics, const Gfx::TopLevelASCreateInfo& createInfo) {
    
}

TopLevelAS::~TopLevelAS() {}