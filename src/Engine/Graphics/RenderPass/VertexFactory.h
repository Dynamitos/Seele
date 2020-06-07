#pragma once
#include "MinimalEngine.h"
#include "Graphics/GraphicsResources.h"

namespace Seele
{
struct VertexInputStream
{
    uint32 streamIndex : 4;
    uint32 offset : 28;
    Gfx::PVertexBuffer vertexBuffer;

    VertexInputStream()
        : streamIndex(0), offset(0), vertexBuffer(nullptr)
    {
    }

    VertexInputStream(uint32 streamIndex, uint32 offset, Gfx::PVertexBuffer vertexBuffer)
        : streamIndex(streamIndex), offset(offset), vertexBuffer(vertexBuffer)
    {
        assert(this->streamIndex == streamIndex && this->offset == offset);
    }

    inline bool operator==(const VertexInputStream &rhs) const
    {
        if (streamIndex != rhs.streamIndex || offset != rhs.offset || vertexBuffer != rhs.vertexBuffer)
        {
            return false;
        }
        return true;
    }
    inline bool operator!=(const VertexInputStream &rhs) const
    {
        return !(*this == rhs);
    }
};

/*struct VertexStreamComponent
{
    const Gfx::PVertexBuffer vertexBuffer = nullptr;

    uint32 streamOffset = 0;

    uint8 offset = 0;

    uint8 stride;

    Gfx::SeFormat type;

    VertexStreamComponent()
    {
    }

    VertexStreamComponent(const Gfx::PVertexBuffer vertexBuffer, uint32 offset, uint32 stride, Gfx::SeFormat type) 
        : vertexBuffer(vertexBuffer)
        , streamOffset(0)
        , offset(offset)
        , stride(stride)
        , type(type)
    {
    }

    VertexStreamComponent(const Gfx::PVertexBuffer vertexBuffer, uint32 streamOffset, uint32 offset, uint32 stride, Gfx::SeFormat type) 
        : vertexBuffer(vertexBuffer)
        , streamOffset(streamOffset)
        , offset(offset)
        , stride(stride)
        , type(type)
    {
    }
};*/

typedef Array<VertexInputStream> VertexInputStreamArray;
struct MeshDescription;
DECLARE_REF(VertexFactory);
class VertexFactory
{
public:
    VertexFactory();
    VertexFactory(MeshDescription description);
    ~VertexFactory();
    void getStreams(VertexInputStreamArray& outVertexStreams) const;
    void getPositionOnlyStream(VertexInputStreamArray& outVertexStreams) const;
    virtual bool supportsTesselation() { return false; }
    Gfx::PVertexDeclaration getDeclaration() const {return declaration;}
    Gfx::PVertexDeclaration getPositionDeclaration() const {return positionDeclaration;}
private:
    static List<PVertexFactory> registeredVertexFactories;
    static std::mutex registeredVertexFactoryLock;
    StaticArray<Gfx::VertexStream, 16> streams;
    StaticArray<Gfx::VertexStream, 16> positionStream;
    Gfx::PVertexDeclaration declaration;
    Gfx::PVertexDeclaration positionDeclaration;
};
DEFINE_REF(VertexFactory);
} // namespace Seele
