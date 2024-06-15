#include "Query.h"
#include "Buffer.h"
#include "Command.h"
#include "Containers/Array.h"
#include "Enums.h"
#include "Graphics.h"
#include "Graphics/Query.h"
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Vulkan;

QueryPool::QueryPool(PGraphics graphics, VkQueryType type, VkQueryPipelineStatisticFlags flags, uint32 resultsStride, uint32 numBuffered)
    : graphics(graphics), flags(flags), numQueries(numBuffered), resultsStride(resultsStride) {
    VkQueryPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queryType = type,
        .queryCount = numBuffered,
        .pipelineStatistics = flags,
    };
    VK_CHECK(vkCreateQueryPool(graphics->getDevice(), &info, nullptr, &handle));
    //VkBufferCreateInfo bufferInfo = {
    //    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    //    .pNext = nullptr,
    //    .flags = 0,
    //    .size = (resultsStride + 1) * numQueries * sizeof(uint64),
    //    .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT,
    //};
    //VmaAllocationCreateInfo allocInfo = {
    //    .flags = VMA_ALLOCATION_CREATE_MAPPED_BIT | VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
    //    .usage = VMA_MEMORY_USAGE_AUTO,
    //    .requiredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    //};
    //resultsAlloc = new BufferAllocation(graphics, "QueryResults", bufferInfo, allocInfo, Gfx::QueueType::GRAPHICS);
    //VK_CHECK(vmaMapMemory(graphics->getAllocator(), resultsAlloc->allocation, (void**) & resultsPtr));
}

QueryPool::~QueryPool() { vkDestroyQueryPool(graphics->getDevice(), handle, nullptr); }

void QueryPool::begin() {
    vkCmdResetQueryPool(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, currentQuery, 1);
    vkCmdBeginQuery(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, currentQuery, 0);
    graphics->getGraphicsCommands()->getCommands()->setPipelineStatisticsFlags(flags);
}

void QueryPool::end() {
    vkCmdEndQuery(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, currentQuery);
    //vkCmdCopyQueryPoolResults(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, currentQuery, 1, resultsAlloc->buffer,
    //                          sizeof(uint64) * currentQuery, resultsStride + sizeof(uint64),
    //                          VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WITH_AVAILABILITY_BIT);
    graphics->getGraphicsCommands()->submitCommands();
    currentQuery = (currentQuery + 1) % numQueries;
}

void QueryPool::getQueryResults(Array<uint64>& results) {
    //uint64 numInts = resultsStride / sizeof(uint64);
    while (currentQuery == pendingQuery)
        ;
    results.resize(resultsStride/ sizeof(uint64));
    vkGetQueryPoolResults(graphics->getDevice(), handle, pendingQuery, 1, resultsStride, results.data(), resultsStride,
                          VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
    //uint64* currentPtr = resultsPtr + (pendingQuery * (numInts + 1));
    //std::cout << pendingQuery << *(currentPtr + numInts) << std::endl;
    //std::memcpy(results.data(), currentPtr, resultsStride);
    pendingQuery = (pendingQuery + 1) % numQueries;
}

OcclusionQuery::OcclusionQuery(PGraphics graphics) : QueryPool(graphics, VK_QUERY_TYPE_OCCLUSION, 0, sizeof(uint64), 16) {}

OcclusionQuery::~OcclusionQuery() {}

void OcclusionQuery::beginQuery() { begin(); }

void OcclusionQuery::endQuery() { end(); }

Gfx::OcclusionResult OcclusionQuery::getResults() {
    Array<uint64> result(1);
    getQueryResults(result);
    return Gfx::OcclusionResult{
        .numFragments = result[0],
    };
}

PipelineStatisticsQuery::PipelineStatisticsQuery(PGraphics graphics)
    : QueryPool(graphics, VK_QUERY_TYPE_PIPELINE_STATISTICS,
                VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT |
                    VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT,
                sizeof(PipelineStatisticsQuery), 16) {}

PipelineStatisticsQuery::~PipelineStatisticsQuery() {}

void PipelineStatisticsQuery::beginQuery() { begin(); }

void PipelineStatisticsQuery::endQuery() { end(); }

Gfx::PipelineStatisticsResult PipelineStatisticsQuery::getResults() {
    Array<uint64> result(9);
    getQueryResults(result);
    return Gfx::PipelineStatisticsResult{
        .inputAssemblyVertices = result[0],
        .inputAssemblyPrimitives = result[1],
        .vertexShaderInvocations = result[2],
        .clippingInvocations = result[3],
        .clippingPrimitives = result[4],
        .fragmentShaderInvocations = result[5],
        .computeShaderInvocations = result[6],
        .taskShaderInvocations = result[7],
        .meshShaderInvocations = result[8],
    };
}