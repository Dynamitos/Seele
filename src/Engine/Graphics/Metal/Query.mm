#include "Query.h"
#include "Buffer.h"
#include "Command.h"
#include "Containers/Array.h"
#include "Enums.h"
#include "Graphics.h"
#include <vulkan/vulkan_core.h>

using namespace Seele;
using namespace Seele::Metal;

QueryPool::QueryPool(PGraphics graphics, const std::string& name)
    : graphics(graphics) {
}

QueryPool::~QueryPool() { }

void QueryPool::begin() {
}

void QueryPool::end() {
}

void QueryPool::getQueryResults(Array<uint64>& results) {
}

OcclusionQuery::OcclusionQuery(PGraphics graphics, const std::string& name)
    : QueryPool(graphics, name) {}

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
    : QueryPool(graphics, name) {}

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
    : QueryPool(graphics, name), numTimestamps(numTimestamps) {
}

TimestampQuery::~TimestampQuery() {}

void TimestampQuery::begin() {
    currentTimestamp = 0;
}

void TimestampQuery::write(Gfx::SePipelineStageFlagBits stage, const std::string& name) {
}

void TimestampQuery::end() {
}

Array<Gfx::Timestamp> TimestampQuery::getResults() {
}