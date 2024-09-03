#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"
#include "Graphics/Pipeline.h"
#include "Graphics/Query.h"

namespace Seele {
class DepthCullingPass : public RenderPass {
  public:
    DepthCullingPass(Gfx::PGraphics graphics, PScene scene);
    DepthCullingPass(DepthCullingPass&&) = default;
    DepthCullingPass& operator=(DepthCullingPass&&) = default;
    virtual ~DepthCullingPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;

  private:
    constexpr static uint64 BLOCK_SIZE = 32;
    struct MipParam {
        uint32 srcMipOffset;
        uint32 dstMipOffset;
        UVector2 srcMipDim;
        UVector2 dstMipDim;
    };

    Array<uint32> mipOffsets;
    Array<UVector2> mipDims;

    Gfx::OShaderBuffer depthMipBuffer;
    Gfx::RenderTargetAttachment depthAttachment;
    Gfx::RenderTargetAttachment visibilityAttachment;
    Gfx::ODescriptorLayout depthAttachmentLayout;
    Gfx::OPipelineLayout depthCullingLayout;
    Gfx::OPipelineStatisticsQuery query;
    Gfx::PTimestampQuery timestamps;

    Gfx::OPipelineLayout depthComputeLayout;
    Gfx::OComputeShader depthInitialReduceShader;
    Gfx::PComputePipeline depthInitialReduce;
    Gfx::OComputeShader depthMipGenShader;
    Gfx::PComputePipeline depthMipGen;
    
    Gfx::PShaderBuffer cullingBuffer;
};
DEFINE_REF(DepthCullingPass)
} // namespace Seele
