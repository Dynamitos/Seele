#include "Graphics.h"
#include "Shader.h"


using namespace Seele::Gfx;

Graphics::Graphics() { shaderCompiler = new ShaderCompiler(this); }

Graphics::~Graphics() {}
