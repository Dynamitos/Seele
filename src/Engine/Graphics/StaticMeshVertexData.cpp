#include "StaticMeshVertexData.h"

using namespace Seele;

extern List<VertexData*> vertexDataList;

StaticMeshVertexData::StaticMeshVertexData(Gfx::PGraphics graphics)
    : VertexData(graphics)
{
    vertexDataList.add(this);
}

StaticMeshVertexData::~StaticMeshVertexData()
{}

