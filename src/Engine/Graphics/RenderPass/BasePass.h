#pragma once
#include "MinimalEngine.h"
#include "RenderPass.h"

namespace Seele
{
DECLARE_REF(CameraActor)
class BasePass : public RenderPass
{
public:
    BasePass(Gfx::PGraphics graphics, PScene scene);
    BasePass(BasePass&&) = default;
    BasePass& operator=(BasePass&&) = default;
    virtual ~BasePass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    Gfx::ORenderTargetAttachment colorAttachment;
    Gfx::OTexture2D colorBuffer;
    Gfx::PTexture2D depthBuffer;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;
    
    Array<Gfx::PDescriptorSet> descriptorSets;
    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    // Set 0: viewParameter, provided by renderpass
    static constexpr uint32 INDEX_VIEW_PARAMS = 0;
    // Set 1: vertex buffers, provided by vertexdata
    static constexpr uint32 INDEX_VERTEX_DATA = 1;
    // Set 2: instance data, provided by vertexdata
    static constexpr uint32 INDEX_SCENE_DATA = 2;
    // Set 3: light environment, provided by lightenv
    static constexpr uint32 INDEX_LIGHT_ENV = 3;
    // Set 4: material data, generated from material
    static constexpr uint32 INDEX_MATERIAL = 4;
    // Set 5: light culling data
    Gfx::ODescriptorLayout lightCullingLayout;
    static constexpr uint32 INDEX_LIGHT_CULLING = 5;
};
DEFINE_REF(BasePass)
} // namespace Seele
