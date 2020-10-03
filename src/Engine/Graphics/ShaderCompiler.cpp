#include "ShaderCompiler.h"
#include "Material/Material.h"
#include "VertexShaderInput.h"

using namespace Seele;
using namespace Seele::Gfx;

ShaderCompiler::ShaderCompiler(PGraphics graphics) 
    : graphics(graphics)
{
    
}

ShaderCompiler::~ShaderCompiler() 
{
    
}

void ShaderCompiler::registerMaterial(PMaterial material) 
{
    for(auto& type : VertexInputType::getTypeList())
    {
        material->createShaders(graphics, Gfx::RenderPassType::DepthPrepass, type);
        material->createShaders(graphics, Gfx::RenderPassType::BasePass, type);
    }
}
    
