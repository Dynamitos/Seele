#pragma once
#include "MinimalEngine.h"
#include "Math/Math.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, Viewport)
DECLARE_NAME_REF(Gfx, Graphics)
DECLARE_NAME_REF(Gfx, RenderPass)
DECLARE_REF(RenderGraph)
class RenderPass
{
public:
    RenderPass(PRenderGraph rendergraph, Gfx::PGraphics graphics, Gfx::PViewport viewport);
    virtual ~RenderPass();
    virtual void updateViewFrame(PViewFrame viewFrame) = 0;
    virtual void beginFrame() = 0;
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
protected:
    _declspec(align(16)) struct ViewParameter
    {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Matrix4 inverseProjectionMatrix;
        Vector4 cameraPosition;
        Vector2 screenDimensions;
    } viewParams;
    Gfx::PRenderPass renderPass;
    PRenderGraph renderGraph;
    Gfx::PGraphics graphics;
    Gfx::PViewport viewport;
};
DEFINE_REF(RenderPass)
} // namespace Seele
