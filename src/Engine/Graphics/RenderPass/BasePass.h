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
    Gfx::PRenderTargetAttachment colorAttachment;
    Gfx::PTexture2D depthBuffer;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;
    
    Array<Gfx::PDescriptorSet> descriptorSets;
    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    // Set 0: viewParameter, provided by renderpass
    // Set 1: light environment, provided by lightenv
    static constexpr uint32 INDEX_LIGHT_ENV = 1;
    // Set 2: light culling data
    static constexpr uint32 INDEX_LIGHT_CULLING = 2;
    Gfx::ODescriptorLayout lightCullingLayout;
    // Set 3: material data, generated from material
    static constexpr uint32 INDEX_MATERIAL = 3;
    // Set 4: vertex buffers, provided by vertexdata
    static constexpr uint32 INDEX_VERTEX_DATA = 4;
    // Set 5: instance data, provided by vertexdata
    static constexpr uint32 INDEX_SCENE_DATA = 5;
};
DEFINE_REF(BasePass)
} // namespace Seele
