#include "Graphics.h"
#include "ShaderCompiler.h"

using namespace Seele::Gfx;

Graphics::Graphics()
{
    shaderCompiler = new ShaderCompiler(this);
}

Graphics::~Graphics()
{
}

PVertexBuffer Graphics::getNullVertexBuffer() 
{
    if(nullVertexBuffer == nullptr)
    {
        VertexBufferCreateInfo createInfo;
        createInfo.numVertices = 1;
        createInfo.vertexSize = sizeof(Math::Vector4);
        Math::Vector4 data =  Math::Vector4(1, 1, 1, 1);
        createInfo.resourceData.data = reinterpret_cast<uint8*>(&data);
        createInfo.resourceData.size = sizeof(Math::Vector4);
        nullVertexBuffer = createVertexBuffer(createInfo);
    }
    return nullVertexBuffer;
}