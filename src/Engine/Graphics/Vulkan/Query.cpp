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

QueryPool::QueryPool(PGraphics graphics, VkQueryType type, VkQueryPipelineStatisticFlags flags, uint32 resultsStride, uint32 numBuffered,
                     const std::string& name)
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
    VkDebugUtilsObjectNameInfoEXT nameInfo = {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT,
        .pNext = nullptr,
        .objectType = VK_OBJECT_TYPE_QUERY_POOL,
        .objectHandle = (uint64)handle,
        .pObjectName = name.c_str(),
    };
    vkSetDebugUtilsObjectNameEXT(graphics->getDevice(), &nameInfo);
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdResetQueryPool(cmd->getHandle(), handle, head, numQueries);
}

QueryPool::~QueryPool() { vkDestroyQueryPool(graphics->getDevice(), handle, nullptr); }

void QueryPool::begin() {
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdResetQueryPool(cmd->getHandle(), handle, head, 1);
    vkCmdBeginQuery(cmd->getHandle(), handle, head, 0);
    cmd->setPipelineStatisticsFlags(flags);
}

void QueryPool::end() {
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdEndQuery(cmd->getHandle(), handle, head);
    graphics->getGraphicsCommands()->submitCommands();
    std::unique_lock l(queryMutex);
    head = (head + 1) % numQueries;
    queryCV.notify_all();
}

void QueryPool::getQueryResults(Array<uint64>& results) {
    while (tail == head) {
        std::unique_lock l(queryMutex);
        queryCV.wait(l);
    }
    results.resize(resultsStride / sizeof(uint64));
    vkGetQueryPoolResults(graphics->getDevice(), handle, tail, 1, resultsStride, results.data(), resultsStride,
                          VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
    tail = (tail + 1) % numQueries;
}

OcclusionQuery::OcclusionQuery(PGraphics graphics, const std::string& name)
    : QueryPool(graphics, VK_QUERY_TYPE_OCCLUSION, 0, sizeof(uint64), 16, name) {}

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

PipelineStatisticsQuery::PipelineStatisticsQuery(PGraphics graphics, const std::string& name)
    : QueryPool(graphics, VK_QUERY_TYPE_PIPELINE_STATISTICS,
                VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_VERTICES_BIT | VK_QUERY_PIPELINE_STATISTIC_INPUT_ASSEMBLY_PRIMITIVES_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_VERTEX_SHADER_INVOCATIONS_BIT | VK_QUERY_PIPELINE_STATISTIC_CLIPPING_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_CLIPPING_PRIMITIVES_BIT | VK_QUERY_PIPELINE_STATISTIC_FRAGMENT_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_COMPUTE_SHADER_INVOCATIONS_BIT |
                    VK_QUERY_PIPELINE_STATISTIC_TASK_SHADER_INVOCATIONS_BIT_EXT |
                    VK_QUERY_PIPELINE_STATISTIC_MESH_SHADER_INVOCATIONS_BIT_EXT,
                sizeof(PipelineStatisticsQuery), 128, name) {}

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

TimestampQuery::TimestampQuery(PGraphics graphics, const std::string& name, uint32 numTimestamps)
    : QueryPool(graphics, VK_QUERY_TYPE_TIMESTAMP, 0, sizeof(uint64), 512 * numTimestamps, name), numTimestamps(numTimestamps) {
    pendingTimestamps.resize(numQueries);
}

TimestampQuery::~TimestampQuery() {}

void TimestampQuery::begin() {
    currentTimestamp = 0;
}

void TimestampQuery::write(Gfx::SePipelineStageFlagBits stage, const std::string& name) {
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    uint32 queryIndex = (head * numTimestamps) + currentTimestamp;
    vkCmdResetQueryPool(cmd->getHandle(), handle, queryIndex, 1);
    vkCmdWriteTimestamp(cmd->getHandle(), cast(stage), handle, queryIndex);
    pendingTimestamps[queryIndex] = name;
    currentTimestamp++;
}

void TimestampQuery::end() {
    std::unique_lock l(queryMutex);
    head = (head + 1) % 512;
    queryCV.notify_all();
}

Array<Gfx::Timestamp> TimestampQuery::getResults() {
    while (head == tail) {
        std::unique_lock l(queryMutex);
        queryCV.wait(l);
    }
    Array<uint64> results(numTimestamps);
    uint32 firstQuery = (tail * numTimestamps);
    vkGetQueryPoolResults(graphics->getDevice(), handle, firstQuery, numTimestamps, numTimestamps * sizeof(uint64), results.data(),
                          resultsStride, VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
    tail = (tail + 1) % 512;
    Array<Gfx::Timestamp> res;
    for (uint64 i = 0; i < numTimestamps; ++i) {
        res.add(Gfx::Timestamp{
            .name = pendingTimestamps[firstQuery + i],
            .time = uint64((results[i] + wrapping) * graphics->getTimestampPeriod()),
        });
    }
    return res;
}