#pragma once
#include "MinimalEngine.h"
#include "GraphicsEnums.h"
#include "GraphicsResources.h"

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

typedef Array<VertexInputStream> VertexInputStreamArray;
struct MeshDescription;
DECLARE_REF(VertexShaderInput);
class VertexShaderInput
{
public:
    VertexShaderInput(std::string name);
    VertexShaderInput(MeshDescription description, std::string name);
    ~VertexShaderInput();
    void getStreams(VertexInputStreamArray& outVertexStreams) const;
    void getPositionOnlyStream(VertexInputStreamArray& outVertexStreams) const;
    virtual bool supportsTesselation() { return false; }
    Gfx::PVertexDeclaration getDeclaration() const {return declaration;}
    Gfx::PVertexDeclaration getPositionDeclaration() const {return positionDeclaration;}
    std::string getName() const { return name; }
    std::string getCode() const { return code; }
private:
    static List<PVertexShaderInput> registeredInputs;
    static std::mutex registeredInputsLock;
    Array<Gfx::VertexAttribute> layout;
    StaticArray<Gfx::VertexStream, 16> streams;
    StaticArray<Gfx::VertexStream, 16> positionStreams;
    Gfx::PVertexDeclaration declaration;
    Gfx::PVertexDeclaration positionDeclaration;
    std::string name;
    std::string code;
};
DEFINE_REF(VertexShaderInput);
} // namespace Seele
