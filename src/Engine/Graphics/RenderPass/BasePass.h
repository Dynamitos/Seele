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
    
    Array<Gfx::PDescriptorSet> descriptorSets;
    PCameraActor source;
    Gfx::OPipelineLayout basePassLayout;
    // Set 0: Light environment
    static constexpr uint32 INDEX_LIGHT_ENV = 0;
    Gfx::OShaderBuffer oLightIndexList;
    Gfx::OTexture oLightGrid;
    Gfx::ODescriptorLayout lightLayout;
    // Set 1: viewParameter
    static constexpr uint32 INDEX_VIEW_PARAMS = 1;
    Gfx::ODescriptorLayout viewLayout;
    Gfx::OUniformBuffer viewParamBuffer;
    // Set 2: materials, generated
    static constexpr uint32 INDEX_MATERIAL = 2;
    // Set 3: primitive scene data
    static constexpr uint32 INDEX_SCENE_DATA = 3;
    Gfx::ODescriptorLayout sceneLayout;
};
DEFINE_REF(BasePass)
} // namespace Seele
