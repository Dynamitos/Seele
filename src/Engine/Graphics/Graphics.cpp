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