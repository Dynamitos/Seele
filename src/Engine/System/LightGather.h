#pragma once
#include "SystemBase.h"

namespace Seele {
namespace System {
class LightGather : public SystemBase {
  public:
    LightGather(PScene scene);
    virtual ~LightGather();
    virtual void update() override;

  private:
    PLightEnvironment lightEnv;
};
} // namespace System
} // namespace Seele