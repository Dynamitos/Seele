#pragma once
#include "Graphics/Buffer.h"
#include "Graphics/Descriptor.h"
#include "RenderPass.h"

namespace Seele {

static constexpr uint64 SHADOW_MAP_SIZE = 8192;
static constexpr uint64 NUM_CASCADES = 4;

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
    struct Cascade {
        Gfx::OTexture2DArray shadowMaps;
        Array<Gfx::OTextureView> views;
        Array<Matrix4> lightSpaceMatrices;
        Gfx::OShaderBuffer lightSpaceBuffer;
        Array<Gfx::PDescriptorSet> viewParams;
    };
    StaticArray<Cascade, NUM_CASCADES> cascades;
    Gfx::OUniformBuffer cascadeSplitsBuffer;
    Gfx::OPipelineLayout shadowLayout;
    Gfx::PShaderBuffer cullingBuffer;
    Gfx::OViewport shadowViewport;
    PScene scene;
};
DEFINE_REF(ShadowPass)
} // namespace Seele