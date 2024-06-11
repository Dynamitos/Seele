#pragma once
#include "MinimalEngine.h"

namespace Seele {
namespace Gfx {
class OcclusionQuery {
  public:
    OcclusionQuery();
    virtual ~OcclusionQuery();
    virtual void beginQuery() = 0;
    virtual void endQuery() = 0;
    virtual void resetQuery() = 0;
    virtual uint64 getResults() = 0;

  private:
};
DEFINE_REF(OcclusionQuery)
} // namespace Gfx
} // namespace Seele