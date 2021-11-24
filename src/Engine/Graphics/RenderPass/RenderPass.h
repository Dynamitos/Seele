#pragma once
#include "MinimalEngine.h"
#include "Math/Math.h"
#include "RenderGraph.h"
#include "ThreadPool.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Viewport)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, RenderPass)
template<typename RenderPassDataType>
class RenderPass
{
public:
    RenderPass(Gfx::PGraphics graphics, Gfx::PViewport viewport)
        : graphics(graphics)
        , viewport(viewport)
    {}
    virtual ~RenderPass()
    {}
    void updateViewFrame(RenderPassDataType viewFrame) {
        passData = std::move(viewFrame);
    }
    virtual Job beginFrame() = 0;
    virtual Job render() = 0;
    virtual Job endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
    void setResources(PRenderGraphResources resources) { this->resources = resources; }
protected:
    struct ViewParameter
    {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Matrix4 inverseProjectionMatrix;
        Vector4 cameraPosition;
        Vector2 screenDimensions;
    } viewParams;
    PRenderGraphResources resources;
    RenderPassDataType passData;
    Gfx::PRenderPass renderPass;
    Gfx::PGraphics graphics;
    Gfx::PViewport viewport;
};
template<typename T>
concept RenderPassType = true;

} // namespace Seele
