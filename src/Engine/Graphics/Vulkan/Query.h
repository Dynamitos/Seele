#pragma once
#include "Graphics.h"
#include "Graphics/Query.h"
#include "Buffer.h"

namespace Seele {
namespace Vulkan {
class QueryPool {
  public:
    QueryPool(PGraphics graphics, VkQueryType type, VkQueryPipelineStatisticFlags flags, uint32 resultsStride, uint32 numBuffered);
    virtual ~QueryPool();
    void begin();
    void end();
    // stalls for the currently first pending query, dont call in render thread
    void getQueryResults(Array<uint64>& results);

  protected:
    PGraphics graphics;
    VkQueryPool handle;
    VkQueryPipelineStatisticFlags flags;
    OBufferAllocation resultsAlloc;
    uint64* resultsPtr;
    uint32 pendingQuery = 0;
    uint32 currentQuery = 0;
    uint32 numQueries;
    uint32 resultsStride;
};
class OcclusionQuery : public Gfx::OcclusionQuery, public QueryPool {
  public:
    OcclusionQuery(PGraphics graphics);
    virtual ~OcclusionQuery();
    virtual void beginQuery() override;
    virtual void endQuery() override;
    virtual Gfx::OcclusionResult getResults() override;
};
DEFINE_REF(OcclusionQuery)
class PipelineStatisticsQuery : public Gfx::PipelineStatisticsQuery, public QueryPool {
  public:
    PipelineStatisticsQuery(PGraphics graphics);
    virtual ~PipelineStatisticsQuery();
    virtual void beginQuery() override;
    virtual void endQuery() override;
    virtual Gfx::PipelineStatisticsResult getResults() override;
};
DEFINE_REF(PipelineStatisticsQuery)

} // namespace Vulkan
} // namespace Seele