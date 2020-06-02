#include "VertexFactory.h"

using namespace Seele;

void VertexFactory::getStreams(VertexInputStreamArray& outVertexStreams) const
{
    for(uint32 i = 0; i < streams.size(); ++i)
    {
        const Gfx::VertexStream& stream = streams[i];
        if(stream.vertexBuffer == nullptr)
        {
            outVertexStreams.add(VertexInputStream(i, 0, nullptr));
        }
        else
        {
            outVertexStreams.add(VertexInputStream(i, stream.offset, stream.vertexBuffer));
        }
    }
}

void VertexFactory::getPositionOnlyStream(VertexInputStreamArray& outVertexStreams) const
{
    for (uint32 i = 0; i < positionStream.size(); ++i)
    {
        const Gfx::VertexStream& stream = positionStream[i];
        outVertexStreams.add(VertexInputStream(i, stream.offset, stream.vertexBuffer));
    }
}