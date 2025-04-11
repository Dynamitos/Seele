#pragma once
#include "Graphics/Pipeline.h"
#include "Graphics/Query.h"
#include "MinimalEngine.h"
#include "RenderPass.h"

namespace Seele {
class DepthCullingPass : public RenderPass {
  public:
    DepthCullingPass(Gfx::PGraphics graphics, PScene scene);
    DepthCullingPass(DepthCullingPass&&) = default;
    DepthCullingPass& operator=(DepthCullingPass&&) = default;
    virtual ~DepthCullingPass();
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    constexpr static uint64 BLOCK_SIZE = 32;
    struct MipParam {
        uint32 sourceOffset;
        uint32 destOffset;
        UVector2 sourceDim;
        UVector2 destDim;
    };

    Array<uint32> mipOffsets;
    Array<UVector2> mipDims;

    constexpr static const char* DEPTHTEXTURE_NAME = "depthTexture";
    Gfx::OShaderBuffer depthMipBuffer;
    constexpr static const char* DEPTHMIP_NAME = "depthMip";
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::ODescriptorLayout depthAttachmentLayout;
    Gfx::OPipelineLayout depthCullingLayout;
    Gfx::OPipelineStatisticsQuery query;
    Gfx::PTimestampQuery timestamps;

    Gfx::OPipelineLayout depthComputeLayout;
    Gfx::OComputeShader depthSourceCopyShader;
    Gfx::PComputePipeline depthSourceCopy;
    Gfx::OComputeShader depthReduceLevelShader;
    Gfx::PComputePipeline depthReduceLevel;

    Gfx::PShaderBuffer cullingBuffer;
    PScene scene;
};
DEFINE_REF(DepthCullingPass)
} // namespace Seele
