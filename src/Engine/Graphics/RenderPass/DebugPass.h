#pragma once
#include "RenderPass.h"
#include "Scene/Scene.h"
#include "Graphics/DebugVertex.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
class DebugPass : public RenderPass
{
public:
    DebugPass(Gfx::PGraphics graphics, PScene scene);
    DebugPass(DebugPass&&) = default;
    DebugPass& operator=(DebugPass&&) = default;
    virtual ~DebugPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::OVertexBuffer debugVertices;
    Gfx::OUniformBuffer viewParamsBuffer;
    Gfx::ODescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
    Gfx::OPipelineLayout pipelineLayout;
    Gfx::OGraphicsPipeline pipeline;
};
DEFINE_REF(DebugPass)
} // namespace Seele
