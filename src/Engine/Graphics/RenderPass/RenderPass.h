#pragma once
#include "MinimalEngine.h"
#include "Math/Math.h"
#include "RenderGraphResources.h"
#include "Component/Camera.h"
#include "Scene/Scene.h"
#include "Material/MaterialInstance.h"
#include "Graphics/VertexData.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Viewport)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, RenderPass)
class RenderPass
{
public:
    RenderPass(Gfx::PGraphics graphics, PScene scene);
    RenderPass(RenderPass&&) = default;
    RenderPass& operator=(RenderPass&&) = default;
    virtual ~RenderPass();
    virtual void beginFrame(const Component::Camera& cam);
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
    void setResources(PRenderGraphResources _resources);
    void setViewport(Gfx::PViewport _viewport);
protected:
    struct ViewParameter
    {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Vector4 cameraPosition;
        Vector2 screenDimensions;
    } viewParams;
    PRenderGraphResources resources;
    Gfx::ODescriptorLayout viewParamsLayout;
    Gfx::OUniformBuffer viewParamsBuffer;
    Gfx::PDescriptorSet viewParamsSet;
    Gfx::ORenderPass renderPass;
    Gfx::PGraphics graphics;
    Gfx::PViewport viewport;
    PScene scene;
};
template<typename RP>
concept RenderPassType = std::derived_from<RP, RenderPass>;

} // namespace Seele
