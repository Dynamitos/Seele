#pragma once
#include "Graphics/Descriptor.h"
#include "MinimalEngine.h"
#include "Graphics/Graphics.h"
#include "Graphics/Shader.h"
#include <filesystem>

namespace Seele
{
DECLARE_REF(EnvironmentMapAsset)
struct EnvironmentImportArgs {
    std::filesystem::path filePath;
    std::string importPath;
};
class EnvironmentLoader {
  public:
    EnvironmentLoader(Gfx::PGraphics graphic);
    ~EnvironmentLoader();
    void importAsset(EnvironmentImportArgs args);

  private:
    void import(EnvironmentImportArgs args, PEnvironmentMapAsset asset);
    Gfx::PGraphics graphics;
    
    Gfx::OTexture2D lutTexture;
    Gfx::OSampler lutSampler;
    Gfx::OViewport lutViewport;
    Gfx::OVertexShader lutVert;
    Gfx::OFragmentShader lutFrag;

    Gfx::OVertexShader cubeRenderVertex;
    Gfx::OFragmentShader cubeRenderFrag;
    Gfx::ODescriptorLayout cubeRenderLayout;
    Gfx::OPipelineLayout cubePipelineLayout;
    
    Gfx::OFragmentShader convolutionFrag;
    
    Gfx::OFragmentShader prefilterFrag;
    
    Gfx::OViewport cubeRenderViewport;
    Gfx::OViewport convolutionViewport;
    // for now we hardcode 128x128 as mip 0, so we have 8 roughness levels
    StaticArray<Gfx::OViewport, 8> prefilterViewports;
    Gfx::OSampler cubeSampler;
};
}