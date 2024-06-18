#pragma once
#include "Resources.h"

namespace Seele {
namespace Gfx {
DECLARE_REF(PipelineLayout)
class RayTracingPipeline
{
  public:
    RayTracingPipeline(PPipelineLayout layout);
    virtual ~RayTracingPipeline();
    PPipelineLayout getPipelineLayout() const;

  protected:
    PPipelineLayout layout;
};
DEFINE_REF(RayTracingPipeline)
class BottomLevelAS {
  public:
    BottomLevelAS();
    ~BottomLevelAS();

  private:
};
DEFINE_REF(BottomLevelAS)
class TopLevelAS {
  public:
    TopLevelAS();
    ~TopLevelAS();

  private:
};
DEFINE_REF(TopLevelAS)
} // namespace Gfx
} // namespace Seele