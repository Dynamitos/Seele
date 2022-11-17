#pragma once
#include "MinimalEngine.h"
#include "Math/Math.h"
#include "RenderGraphResources.h"
#include "Component/Camera.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Viewport)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, RenderPass)
template<typename RenderPassDataType>
class RenderPass
{
public:
    RenderPass(Gfx::PGraphics graphics)
        : graphics(graphics)
    {}
    virtual ~RenderPass()
    {}
    void updateViewFrame(RenderPassDataType viewFrame) {
        passData = std::move(viewFrame);
    }
    virtual void beginFrame(const Component::Camera& cam) = 0;
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
    void setResources(PRenderGraphResources _resources) { resources = _resources; }
    void setViewport(Gfx::PViewport _viewport) 
    { 
        viewport = _viewport;
        publishOutputs();
    }
protected:
    struct ViewParameter
    {
        Math::Matrix4 viewMatrix;
        Math::Matrix4 projectionMatrix;
        Math::Vector4 cameraPosition;
        Math::Vector2 screenDimensions;
        Math::Vector2 pad0;
    } viewParams;
    PRenderGraphResources resources;
    RenderPassDataType passData;
    Gfx::PRenderPass renderPass;
    Gfx::PGraphics graphics;
    Gfx::PViewport viewport;
};
template<typename RP, typename T>
concept RenderPassType = std::derived_from<RP, RenderPass<T>>;

} // namespace Seele
