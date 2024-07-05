#pragma once
#include "Containers/Array.h"
#include "Enums.h"
#include "MinimalEngine.h"
#include <ostream>
#include <chrono>
#include <fmt/format.h>
#include <string>

namespace Seele {
namespace Gfx {
struct OcclusionResult {
    uint64 numFragments;
};
class OcclusionQuery {
  public:
    OcclusionQuery();
    virtual ~OcclusionQuery();
    virtual void beginQuery() = 0;
    virtual void endQuery() = 0;
    virtual OcclusionResult getResults() = 0;
};
DEFINE_REF(OcclusionQuery)
struct PipelineStatisticsResult {
    uint64 inputAssemblyVertices;
    uint64 inputAssemblyPrimitives;
    uint64 vertexShaderInvocations;
    uint64 clippingInvocations;
    uint64 clippingPrimitives;
    uint64 fragmentShaderInvocations;
    uint64 computeShaderInvocations;
    uint64 taskShaderInvocations;
    uint64 meshShaderInvocations;
    friend std::ostream& operator<<(std::ostream& buf, const PipelineStatisticsResult& res);
};

static std::ostream& operator<<(std::ostream & buf, const PipelineStatisticsResult& res) {
    buf << fmt::format("{},{},{},{},{},{},{},{},{},", res.inputAssemblyVertices, res.inputAssemblyPrimitives, res.vertexShaderInvocations,
                       res.clippingInvocations, res.clippingPrimitives, res.fragmentShaderInvocations, res.computeShaderInvocations, res.taskShaderInvocations,
                       res.meshShaderInvocations);
    return buf;
}
class PipelineStatisticsQuery {
  public:
    PipelineStatisticsQuery();
    virtual ~PipelineStatisticsQuery();
    virtual void beginQuery() = 0;
    virtual void endQuery() = 0;
    virtual PipelineStatisticsResult getResults() = 0;
};
DEFINE_REF(PipelineStatisticsQuery)
struct Timestamp {
    std::string name;
    uint64 time;
};
class TimestampQuery {
  public:
    TimestampQuery();
    virtual ~TimestampQuery();
    virtual void begin() = 0;
    virtual void write(SePipelineStageFlagBits stage, const std::string& name = "") = 0;
    virtual void end() = 0;
    virtual Array<Timestamp> getResults() = 0;
};
DEFINE_REF(TimestampQuery)
} // namespace Gfx
} // namespace Seele