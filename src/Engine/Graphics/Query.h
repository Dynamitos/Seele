#pragma once
#include "MinimalEngine.h"

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
};
class PipelineStatisticsQuery {
  public:
    PipelineStatisticsQuery();
    virtual ~PipelineStatisticsQuery();
    virtual void beginQuery() = 0;
    virtual void endQuery() = 0;
    virtual PipelineStatisticsResult getResults() = 0;
};
DEFINE_REF(PipelineStatisticsQuery)
} // namespace Gfx
} // namespace Seele