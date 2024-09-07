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
    : graphics(graphics), type(type), name(name), flags(flags), numQueries(numBuffered), resultsStride(resultsStride) {
    createPool();
}

QueryPool::~QueryPool() {
    for (auto handle : pools) {
        vkDestroyQueryPool(graphics->getDevice(), handle, nullptr);
    }
}

void QueryPool::begin() {
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdResetQueryPool(cmd->getHandle(), pools.back(), head, 1);
    vkCmdBeginQuery(cmd->getHandle(), pools.back(), head, 0);
    cmd->setPipelineStatisticsFlags(flags);
}

void QueryPool::end() {
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdEndQuery(cmd->getHandle(), pools.back(), head);
    std::unique_lock l(queryMutex);
    head++;
    if (head == numQueries) {
        createPool();
        head = 0;
    }
    numAvailable++;
    queryCV.notify_all();
}

void QueryPool::getQueryResults(Array<uint64>& results) {
    {
        std::unique_lock l(queryMutex);
        while (numAvailable == 0) {
            queryCV.wait(l);
        } 
    }
    results.resize(resultsStride / sizeof(uint64));
    vkGetQueryPoolResults(graphics->getDevice(), pools.front(), tail, 1, resultsStride, results.data(), resultsStride,
                          VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
    std::unique_lock l(queryMutex);
    tail++;
    if (tail == numQueries)
    {
        vkDestroyQueryPool(graphics->getDevice(), pools.front(), nullptr);
        pools.popFront();
        tail = 0;
    }
    numAvailable--;
}

void QueryPool::createPool() {
    VkQueryPool handle;
    VkQueryPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queryType = type,
        .queryCount = numQueries,
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
    pools.add(handle);
    if (pools.size() > 5)
    {
        vkDestroyQueryPool(graphics->getDevice(), pools.front(), nullptr);
        pools.popFront();
    }
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
                sizeof(Gfx::PipelineStatisticsResult), 128, name) {}

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

TimestampQuery::TimestampQuery(PGraphics graphics, const std::string& name)
    : QueryPool(graphics, VK_QUERY_TYPE_TIMESTAMP, 0, sizeof(uint64), 512, name) {
}

TimestampQuery::~TimestampQuery() {}

void TimestampQuery::write(Gfx::SePipelineStageFlagBits stage, const std::string& name) {
    PCommand cmd = graphics->getGraphicsCommands()->getCommands();
    vkCmdResetQueryPool(cmd->getHandle(), pools.back(), head, 1);
    vkCmdWriteTimestamp(cmd->getHandle(), cast(stage), pools.back(), head);
    pendingTimestamps.add(name);
    {
        std::unique_lock l(queryMutex);
        head++;
        if (head == numQueries) {
            createPool();
            head = 0;
        }
        numAvailable++;
    }
}

Gfx::Timestamp TimestampQuery::getResult() {
    {
        std::unique_lock l(queryMutex);
        while (numAvailable == 0) {
            queryCV.wait(l);
        } 
    }
    uint64 result;
    vkGetQueryPoolResults(graphics->getDevice(), pools.front(), tail, 1, sizeof(uint64), &result, resultsStride,
                          VK_QUERY_RESULT_WAIT_BIT | VK_QUERY_RESULT_64_BIT);
    {
        std::unique_lock l(queryMutex);
        tail++;
        if (tail == numQueries) {
            vkDestroyQueryPool(graphics->getDevice(), pools.front(), nullptr);
            pools.popFront();
            tail = 0;
        }
        numAvailable--;
        Gfx::Timestamp res = Gfx::Timestamp{
            .name = pendingTimestamps.front(),
            .time = result,
        };
        pendingTimestamps.popFront();
        return res;
    }
}