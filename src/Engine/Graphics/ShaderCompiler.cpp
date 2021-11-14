#include "ShaderCompiler.h"
#include "Material/MaterialAsset.h"
#include "VertexShaderInput.h"
#include "Graphics.h"

using namespace Seele;
using namespace Seele::Gfx;

ShaderCompiler::ShaderCompiler(PGraphics graphics) 
    : graphics(graphics)
{
    
}

ShaderCompiler::~ShaderCompiler() 
{
    
}

void ShaderCompiler::registerMaterial(PMaterialAsset material) 
{
    for(auto& type : VertexInputType::getTypeList())
    {
        material->createShaders(graphics, Gfx::RenderPassType::DepthPrepass, type);
        material->createShaders(graphics, Gfx::RenderPassType::BasePass, type);
    }
}
    
