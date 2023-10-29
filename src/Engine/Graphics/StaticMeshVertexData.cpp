#include "StaticMeshVertexData.h"

using namespace Seele;

extern List<VertexData*> vertexDataList;

StaticMeshVertexData::StaticMeshVertexData(Gfx::PGraphics graphics)
    : VertexData(graphics)
    , head(0)
{
    vertexDataList.add(this);
}

StaticMeshVertexData::~StaticMeshVertexData()
{}

MeshId StaticMeshVertexData::allocateVertexData(uint64 numVertices)
{
    MeshId res{idCounter++};
    meshOffsets[res] = head;
    head += numVertices;
    return res;
}
