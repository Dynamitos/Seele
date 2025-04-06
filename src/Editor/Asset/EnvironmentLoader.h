#pragma once
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
    Gfx::OVertexShader cubeRenderVertex;
    Gfx::OFragmentShader cubeRenderFrag;
    Gfx::ODescriptorLayout cubeRenderLayout;
    Gfx::OPipelineLayout cubePipelineLayout;
    Gfx::PGraphicsPipeline cubeRenderPipeline;
    Gfx::OFragmentShader convolutionFrag;
    Gfx::PGraphicsPipeline convolutionPipeline;
    Gfx::OViewport cubeRenderViewport;
    Gfx::OViewport convolutionViewport;
    Gfx::OSampler cubeSampler;
};
}