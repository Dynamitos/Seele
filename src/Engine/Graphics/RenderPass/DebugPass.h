#pragma once
#include "RenderPass.h"
#include "Graphics/GraphicsResources.h"
#include "Scene/Scene.h"
#include "Graphics/DebugVertex.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
struct DebugPassData
{
    Array<DebugVertex> vertices;
};
class DebugPass : public RenderPass<DebugPassData>
{
public:
    DebugPass(Gfx::PGraphics graphics);
    virtual ~DebugPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    Gfx::PVertexBuffer debugVertices;
    Gfx::PUniformBuffer viewParamsBuffer;
    Gfx::PDescriptorLayout descriptorLayout;
    Gfx::PDescriptorSet descriptorSet;
    Gfx::PPipelineLayout pipelineLayout;
    Gfx::PGraphicsPipeline pipeline;
};
DEFINE_REF(DebugPass)
} // namespace Seele
