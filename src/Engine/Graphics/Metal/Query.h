#pragma once
#include "Buffer.h"
#include "Graphics.h"
#include "Graphics/Query.h"

namespace Seele {
namespace Metal {
class QueryPool {
  public:
    QueryPool(PGraphics graphics, const std::string& name);
    virtual ~QueryPool();
    void begin();
    void end();
    // stalls for the currently first pending query, dont call in render thread
    void getQueryResults(Array<uint64>& results);

  protected:
    PGraphics graphics;
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
    virtual void write(Gfx::SePipelineStageFlagBits stage, const std::string& name = "") override;
    virtual Gfx::Timestamp getResult() override;

  private:
    Array<std::string> pendingTimestamps;
};
DEFINE_REF(TimestampQuery)
} // namespace Vulkan
} // namespace Seele