#pragma once
#include "GraphicsResources.h"
#include "VertexData.h"

namespace Seele
{
class TopologyData
{
public:
    TopologyData();
    virtual ~TopologyData();
    virtual void bind() = 0;
protected:
    VertexData* vertexData;
};
DEFINE_REF(TopologyData)
} // namespace Seele