#pragma once
#include "RenderPass.h"

namespace Seele {
class TerrainRenderer {
  public:
    TerrainRenderer(Gfx::PGraphics graphics, PScene scene, Gfx::PDescriptorLayout viewParamsLayout);
    ~TerrainRenderer();
    void beginFrame();
    Gfx::ORenderCommand render(Gfx::PDescriptorSet viewParamsSet);
    void setViewport(Gfx::PViewport viewport, Gfx::PRenderPass renderPass);

  private:
    Gfx::PGraphics graphics;
    PScene scene;
    Gfx::ODescriptorLayout layout;
    Gfx::PDescriptorSet set;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::OTaskShader task;
    Gfx::OMeshShader mesh;
    Gfx::OFragmentShader frag;
    Gfx::PGraphicsPipeline pipeline;
    Gfx::PViewport viewport;
    Gfx::OShaderBuffer tilesBuffer;
    Gfx::OTexture2D displacementMap;
    Gfx::OSampler sampler;
};
DEFINE_REF(TerrainRenderer);
}