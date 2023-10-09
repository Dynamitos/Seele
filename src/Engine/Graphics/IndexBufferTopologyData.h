#pragma once
#include "TopologyData.h"

namespace Seele
{
class IndexBufferTopologyData : public TopologyData
{
public:
    IndexBufferTopologyData();
    virtual ~IndexBufferTopologyData();
private:
    Gfx::PIndexBuffer indices;
};
} // namespace Seele