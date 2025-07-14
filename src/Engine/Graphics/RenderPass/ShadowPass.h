#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Descriptor.h"
#include "RenderPass.h"

namespace Seele {

static constexpr uint64 SHADOW_MAP_SIZE = 8192;

class ShadowPass : public RenderPass {
  public:
    ShadowPass(Gfx::PGraphics graphics, PScene scene);
    ShadowPass(ShadowPass&& other) = default;
    ShadowPass& operator=(ShadowPass&& other) = default;
    virtual ~ShadowPass();
    virtual void beginFrame(const Component::Camera& cam, const Component::Transform& transform) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
  private:
    AABB cameraFrustumBox;
    static constexpr uint64 NUM_CASCADES = 4;
    struct Cascade {
        Gfx::OTexture2D shadowMaps;
        Array<Gfx::OTextureView> views;
        Array<Gfx::PDescriptorSet> viewParams;
    };
    StaticArray<Cascade, NUM_CASCADES> cascades;
    Gfx::OPipelineLayout shadowLayout;
    Gfx::PShaderBuffer cullingBuffer;
    Gfx::OViewport shadowViewport;
    PScene scene;
};
DEFINE_REF(ShadowPass)
} // namespace Seele