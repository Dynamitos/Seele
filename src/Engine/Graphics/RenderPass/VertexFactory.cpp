#include "VertexFactory.h"
#include "Graphics/Mesh.h"
#include <memory>

using namespace Seele;

List<PVertexFactory> VertexFactory::registeredVertexFactories;
std::mutex VertexFactory::registeredVertexFactoryLock;

VertexFactory::VertexFactory() 
{
    std::scoped_lock lock(registeredVertexFactoryLock);
    registeredVertexFactories.add(this);
}

VertexFactory::VertexFactory(MeshDescription description)
    : declaration(description.declaration)
{
    auto declStreams = declaration->getVertexStreams();
    std::copy(declStreams.begin(), declStreams.end(), streams.begin());

    uint32 positionStreamIndex = 0;
    positionDeclaration = new Gfx::VertexDeclaration();
    for (uint32 i = 0; i < declStreams.size(); i++)
    {
        if(description.layout[i] == VertexAttribute::POSITION)
        {
            positionStream[positionStreamIndex++] = declStreams[i];
            positionDeclaration->addVertexStream(declStreams[i]);
        }
    }
    
    std::scoped_lock lock(registeredVertexFactoryLock);
    registeredVertexFactories.add(this);
}

VertexFactory::~VertexFactory()
{
    registeredVertexFactories.remove(registeredVertexFactories.find(this));
}

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