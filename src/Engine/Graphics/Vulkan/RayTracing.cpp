#include "RayTracing.h"
#include "Buffer.h"
#include "Graphics/Initializer.h"
#include <vulkan/vulkan_core.h>

using namespace Seele::Vulkan;

BottomLevelAS::BottomLevelAS(PGraphics graphics, const Gfx::BottomLevelASCreateInfo& createInfo)
{
    VkDeviceOrHostAddressConstKHR vertexDataDeviceAddress = {
        .deviceAddress = createInfo.positionBuffer.cast<Buffer>()->getDeviceAddress() + createInfo.verticesOffset,
    };
    VkDeviceOrHostAddressConstKHR indexDataDeviceAddress = {
        .deviceAddress = createInfo.indexBuffer.cast<Buffer>()->getDeviceAddress() + createInfo.indicesOffset,
    };
    VkAccelerationStructureGeometryKHR accelerationStructureGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
        .pNext = nullptr,
        .geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR,
        .geometry = {.triangles = {
            .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
            .pNext = nullptr,
            .vertexFormat = VK_FORMAT_R32G32B32_SFLOAT,
            .vertexData = vertexDataDeviceAddress,
            .vertexStride = sizeof(Vector),
            .maxVertex = createInfo.positionBuffer->getNumElements(),
            .indexType = VK_INDEX_TYPE_UINT32,
            .indexData = indexDataDeviceAddress,
        }},
        .flags = VK_GEOMETRY_OPAQUE_BIT_KHR,
    };
    
    VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
        .primitiveCount = createInfo.meshData.numIndices / 3,
        .primitiveOffset = 0,
        .firstVertex = 0,
        .transformOffset = 0
    };

    VkAccelerationStructureBuildGeometryInfoKHR buildGeometry = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
        .pNext = nullptr,
        .type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
        .flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
    };

    VkAccelerationStructureCreateInfoKHR info = {
        .sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
        .pNext = nullptr,
        .createFlags = 0,
    };
}

BottomLevelAS::~BottomLevelAS()
{

}

TopLevelAS::TopLevelAS(PGraphics graphics, const Gfx::TopLevelASCreateInfo& createInfo)
{

}

TopLevelAS::~TopLevelAS()
{

}