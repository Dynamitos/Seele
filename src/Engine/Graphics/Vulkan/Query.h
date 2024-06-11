#pragma once
#include "Enums.h"
#include "Graphics.h"
#include "Graphics/Query.h"

namespace Seele {
namespace Vulkan {
class OcclusionQuery : public Gfx::OcclusionQuery {
  public:
    OcclusionQuery(PGraphics graphics);
    virtual ~OcclusionQuery();
    virtual void beginQuery() override;
    virtual void endQuery() override;
    virtual void resetQuery() override;
    virtual uint64 getResults() override;
  private:
    PGraphics graphics;
    VkQueryPool handle;
};
DEFINE_REF(OcclusionQuery)
} // namespace Vulkan
} // namespace Seele