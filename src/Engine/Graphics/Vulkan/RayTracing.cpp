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
    matrix = {
        //    createInfo.mesh->transform[0][0], createInfo.mesh->transform[0][1], createInfo.mesh->transform[0][2],
        //    createInfo.mesh->transform[0][3], createInfo.mesh->transform[1][0], createInfo.mesh->transform[1][1],
        //    createInfo.mesh->transform[1][2], createInfo.mesh->transform[1][3], createInfo.mesh->transform[2][0],
        //    createInfo.mesh->transform[2][1], createInfo.mesh->transform[2][2], createInfo.mesh->transform[2][3],
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
    };
    VertexData* vertexData = createInfo.mesh->vertexData;
    MeshData meshData = vertexData->getMeshData(createInfo.mesh->id);
    vertexOffset = vertexData->getMeshOffset(createInfo.mesh->id) * sizeof(Vector4);
    indexOffset = meshData.firstIndex * sizeof(uint32);
    primitiveCount = meshData.numIndices / 3;

    // todo: compact
}

BottomLevelAS::~BottomLevelAS() { graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffer)); }

TopLevelAS::TopLevelAS(PGraphics graphics, const Gfx::TopLevelASCreateInfo& createInfo) : graphics(graphics) {
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
            .instanceShaderBindingTableRecordOffset = 0,
            .flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
            .accelerationStructureReference = blas->getDeviceAddress(),
        };
    }

    instanceAllocation = new BufferAllocation(graphics, "ASInstances",
                                              VkBufferCreateInfo{
                                                  .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
                                                  .pNext = nullptr,
                                                  .flags = 0,
                                                  .size = sizeof(VkAccelerationStructureInstanceKHR) * instances.size(),
                                                  .usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
                                                           VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
                                              },
                                              VmaAllocationCreateInfo{
                                                  .usage = VMA_MEMORY_USAGE_AUTO,
                                              },
                                              Gfx::QueueType::GRAPHICS);
    instanceAllocation->updateContents(0, sizeof(VkAccelerationStructureInstanceKHR) * instances.size(), instances.data());
    VkDeviceOrHostAddressConstKHR instanceDeviceAddress = {
        .deviceAddress = instanceAllocation->deviceAddress,
    };

    VkAccelerationStructureGeometryKHR geometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR,
        .geometry =
            {
                .instances =
                    {
                        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
                        .pNext = nullptr,
                        .arrayOfPointers = VK_FALSE,
                        .data = instanceDeviceAddress,
                    },
            },
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };
    VkAccelerationStructureBuildGeometryInfoKHR structureBuildGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext = nullptr,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
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
    VkBufferMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
        .pNext = nullptr,
        .srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR,
        .dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .buffer = buffer->buffer,
        .offset = 0,
        .size = buffer->size,
    };
    vkCmdPipelineBarrier(cmd->getHandle(), VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR, 0, 0,
                         nullptr, 1, &barrier, 0, nullptr);
    scratchBuffer->bind();
    cmd->bindResource(PBufferAllocation(scratchBuffer));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(scratchBuffer));

    buffer->bind();
    cmd->bindResource(PBufferAllocation(buffer));

    instanceAllocation->bind();
    cmd->bindResource(PBufferAllocation(instanceAllocation));
}

TopLevelAS::~TopLevelAS() {
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(buffer));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(instanceAllocation));
}

RayTracingPipeline::RayTracingPipeline(PGraphics graphics, VkPipeline handle, OBufferAllocation rayGen, uint64 rayGenStride,
                                       OBufferAllocation hit, uint64 hitStride, OBufferAllocation miss, uint64 missStride,
                                       OBufferAllocation callable, uint64 callableStride, Gfx::PPipelineLayout layout)
    : Gfx::RayTracingPipeline(layout), graphics(graphics), pipeline(handle), rayGen(std::move(rayGen)), rayGenStride(rayGenStride),
      hit(std::move(hit)), hitStride(hitStride), miss(std::move(miss)), missStride(missStride), callable(std::move(callable)),
      callableStride(callableStride) {}

RayTracingPipeline::~RayTracingPipeline() {
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(rayGen));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(hit));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(miss));
    graphics->getDestructionManager()->queueResourceForDestruction(std::move(callable));
}

void RayTracingPipeline::bind(VkCommandBuffer handle) { vkCmdBindPipeline(handle, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline); }
