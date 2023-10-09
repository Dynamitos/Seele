#pragma once
#include "GraphicsResources.h"

namespace Seele
{
class TopologyData
{
public:
    TopologyData();
    virtual ~TopologyData();
    virtual void bind() = 0;
private:
};
} // namespace Seele