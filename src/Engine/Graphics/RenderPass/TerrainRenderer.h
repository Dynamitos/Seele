#pragma once
#include "RenderPass.h"
#include "Graphics/Buffer.h"
#include "Graphics/CBT/CBT.h"

namespace Seele {
class TerrainRenderer {
  public:
    TerrainRenderer(Gfx::PGraphics graphics, PScene scene, Gfx::PDescriptorLayout viewParamsLayout, Gfx::PDescriptorSet viewParamsSet);
    ~TerrainRenderer();
    void beginFrame(Gfx::PDescriptorSet viewParamsSet, const Component::Camera& cam);
    Gfx::ORenderCommand render(Gfx::PDescriptorSet viewParamsSet);
    void setViewport(Gfx::PViewport viewport, Gfx::PRenderPass renderPass);

  private:
    void applyDeformation(Gfx::PDescriptorSet viewParamsSet);
    Gfx::PGraphics graphics;
    PScene scene;
    Gfx::PDescriptorLayout viewParamsLayout;
    Gfx::PRenderPass renderPass;
    Gfx::OTaskShader task;
    Gfx::OMeshShader mesh;
    Gfx::OVertexInput inp;
    Gfx::OVertexShader vert;
    Gfx::OFragmentShader frag;
    Gfx::PGraphicsPipeline pipeline;
    Gfx::OComputeShader deformCS;
    Gfx::PComputePipeline deform;
    Gfx::PViewport viewport;
    Gfx::OShaderBuffer tilesBuffer;
    Gfx::PTexture2D displacementMap;
    Gfx::PTexture2D colorMap;
    Gfx::OSampler sampler;

    Gfx::OUniformBuffer geometryBuffer;
    Gfx::OUniformBuffer updateBuffer;
    BaseMesh baseMesh;
    CBTMesh plainMesh;
    MeshUpdater meshUpdater;
    LebMatrixCache lebCache;
};
DEFINE_REF(TerrainRenderer);
} // namespace Seele