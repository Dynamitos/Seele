#pragma once
#include "Graphics/Query.h"
#include "Math/Vector.h"
#include "RenderPass.h"

namespace Seele {
class VisibilityPass : public RenderPass {
  public:
    VisibilityPass(Gfx::PGraphics graphics);
    VisibilityPass(VisibilityPass&&) = default;
    VisibilityPass& operator=(VisibilityPass&&) = default;
    virtual ~VisibilityPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    static constexpr uint32 BLOCK_SIZE = 32;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::PDescriptorSet visibilitySet;
    Gfx::ODescriptorLayout visibilityDescriptor;
    Gfx::OPipelineLayout visibilityLayout;
    Gfx::OComputeShader visibilityShader;
    Gfx::PComputePipeline visibilityPipeline;
    Gfx::OPipelineStatisticsQuery query;
    Gfx::PTimestampQuery timestamps;

    constexpr static std::string VISIBILITY_NAME = "visibilityTexture";
    // Holds culling information for every meshlet for each instance
    Gfx::OShaderBuffer cullingBuffer;
    constexpr static std::string CULLINGBUFFER_NAME = "cullingBuffer";
    UVector threadGroupSize;
};
DEFINE_REF(VisibilityPass)
} // namespace Seele