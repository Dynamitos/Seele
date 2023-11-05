#include "Graphics.h"
#include "Shader.h"
#include "Graphics.h"

using namespace Seele::Gfx;

Graphics::Graphics()
{
    shaderCompiler = new ShaderCompiler(this);
}

Graphics::~Graphics()
{
}

