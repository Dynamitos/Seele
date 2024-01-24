#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"

namespace Seele
{
class DepthPrepass : public RenderPass
{
public:
    DepthPrepass(Gfx::PGraphics graphics, PScene scene);
    DepthPrepass(DepthPrepass&&) = default;
    DepthPrepass& operator=(DepthPrepass&&) = default;
    virtual ~DepthPrepass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    Gfx::ORenderTargetAttachment depthAttachment;
    Gfx::OTexture2D depthBuffer;

    Array<Gfx::PDescriptorSet> descriptorSets;
    
    Gfx::OPipelineLayout depthPrepassLayout;
    // Set 0: viewParameter
    static constexpr uint32 INDEX_VIEW_PARAMS = 0;
    // Set 0: vertices, from VertexData
    constexpr static uint32 INDEX_VERTEX_DATA = 1;
    // Set 2: mesh data, either index buffer or meshlet data
    constexpr static uint32 INDEX_SCENE_DATA = 2;
    Gfx::ODescriptorLayout sceneDataLayout;
};
DEFINE_REF(DepthPrepass)
} // namespace Seele
