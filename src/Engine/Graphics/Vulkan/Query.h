#pragma once
#include "Buffer.h"
#include "Graphics.h"
#include "Graphics/Query.h"

namespace Seele {
namespace Vulkan {
class QueryPool {
  public:
    QueryPool(PGraphics graphics, VkQueryType type, VkQueryPipelineStatisticFlags flags, uint32 resultsStride, uint32 numBuffered,
              const std::string& name);
    virtual ~QueryPool();
    void begin();
    void end();
    // stalls for the currently first pending query, dont call in render thread
    void getQueryResults(Array<uint64>& results);

  protected:
    PGraphics graphics;
    VkQueryPool handle;
    VkQueryPipelineStatisticFlags flags;
    // ring buffer
    uint64 head = 0;
    uint64 tail = 0;
    uint32 numQueries;
    uint32 resultsStride;
    std::mutex queryMutex;
    std::condition_variable queryCV;
};
class OcclusionQuery : public Gfx::OcclusionQuery, public QueryPool {
  public:
    OcclusionQuery(PGraphics graphics, const std::string& name);
    virtual ~OcclusionQuery();
    virtual void beginQuery() override;
    virtual void endQuery() override;
    virtual Gfx::OcclusionResult getResults() override;
};
DEFINE_REF(OcclusionQuery)
class PipelineStatisticsQuery : public Gfx::PipelineStatisticsQuery, public QueryPool {
  public:
    PipelineStatisticsQuery(PGraphics graphics, const std::string& name);
    virtual ~PipelineStatisticsQuery();
    virtual void beginQuery() override;
    virtual void endQuery() override;
    virtual Gfx::PipelineStatisticsResult getResults() override;
};
DEFINE_REF(PipelineStatisticsQuery)

class TimestampQuery : public Gfx::TimestampQuery, public QueryPool {
  public:
    TimestampQuery(PGraphics graphics, const std::string& name, uint32 numTimestamps);
    virtual ~TimestampQuery();
    virtual void begin() override;
    virtual void write(Gfx::SePipelineStageFlagBits stage, const std::string& name = "") override;
    virtual void end() override;
    virtual Array<Gfx::Timestamp> getResults() override;

  private:
    uint64 wrapping = 0;
    uint64 lastMeasure = 0;
    uint32 numTimestamps = 0;
    uint32 currentTimestamp = 0;
    Array<std::string> pendingTimestamps;
};
DEFINE_REF(TimestampQuery)
} // namespace Vulkan
} // namespace Seele