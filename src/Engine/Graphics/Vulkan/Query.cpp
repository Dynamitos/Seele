#include "Query.h"
#include "Command.h"

using namespace Seele;
using namespace Seele::Vulkan;

OcclusionQuery::OcclusionQuery(PGraphics graphics) :graphics(graphics){
    VkQueryPoolCreateInfo info = {
        .sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .queryType = VK_QUERY_TYPE_OCCLUSION,
        .queryCount = 1,
        .pipelineStatistics = 0,
    };
    VK_CHECK(vkCreateQueryPool(graphics->getDevice(), &info, nullptr, &handle));
}

OcclusionQuery::~OcclusionQuery() { vkDestroyQueryPool(graphics->getDevice(), handle, nullptr); }

void OcclusionQuery::beginQuery() { vkCmdBeginQuery(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, 0, VK_QUERY_CONTROL_PRECISE_BIT); }

void OcclusionQuery::endQuery() { vkCmdEndQuery(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, 0); }

void OcclusionQuery::resetQuery() { vkCmdResetQueryPool(graphics->getGraphicsCommands()->getCommands()->getHandle(), handle, 0, 1); }

uint64 OcclusionQuery::getResults() {
    graphics->getGraphicsCommands()->submitCommands();
    uint64 result;
    vkGetQueryPoolResults(graphics->getDevice(), handle, 0, 1, sizeof(uint64), &result, sizeof(uint64), VK_QUERY_RESULT_64_BIT | VK_QUERY_RESULT_WAIT_BIT);
    return result;
}