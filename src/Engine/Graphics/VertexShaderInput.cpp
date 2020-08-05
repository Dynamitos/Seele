#include "VertexShaderInput.h"
#include "Graphics/Mesh.h"
#include <sstream>
#include <memory>

using namespace Seele;

List<PVertexShaderInput> VertexShaderInput::registeredInputs;
std::mutex VertexShaderInput::registeredInputsLock;

VertexShaderInput::VertexShaderInput(std::string name) 
    : name(name)
{
    std::scoped_lock lock(registeredInputsLock);
    registeredInputs.add(this);
}

VertexShaderInput::VertexShaderInput(MeshDescription description, std::string name)
    : declaration(description.declaration)
    , layout(description.layout)
    , name(name)
{
    auto declStreams = declaration->getVertexStreams();
    std::copy(declStreams.begin(), declStreams.end(), streams.begin());

    uint32 positionStreamIndex = 0;
    positionDeclaration = new Gfx::VertexDeclaration();
    for (uint32 i = 0; i < declStreams.size(); i++)
    {
        if(description.layout[i] == Gfx::VertexAttribute::POSITION)
        {
            positionStreams[positionStreamIndex++] = declStreams[i];
            positionDeclaration->addVertexStream(declStreams[i]);
        }
    }
    
    std::scoped_lock lock(registeredInputsLock);
    registeredInputs.add(this);
}

VertexShaderInput::~VertexShaderInput()
{
    registeredInputs.remove(registeredInputs.find(this));
}

void VertexShaderInput::getStreams(VertexInputStreamArray& outVertexStreams) const
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

void VertexShaderInput::getPositionOnlyStream(VertexInputStreamArray& outVertexStreams) const
{
    for (uint32 i = 0; i < positionStreams.size(); ++i)
    {
        const Gfx::VertexStream& stream = positionStreams[i];
        outVertexStreams.add(VertexInputStream(i, stream.offset, stream.vertexBuffer));
    }
}