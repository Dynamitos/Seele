#pragma once
#include "MinimalEngine.h"
#include "Math/Math.h"

namespace Seele
{
DECLARE_NAME_REF(Gfx, RenderPass)
DECLARE_REF(RenderGraph)
class RenderPass
{
public:
    RenderPass(PRenderGraph rendergraph);
    virtual ~RenderPass();
    virtual void beginFrame() = 0;
    virtual void render() = 0;
    virtual void endFrame() = 0;
    virtual void publishOutputs() = 0;
    virtual void createRenderPass() = 0;
protected:
    struct ViewParameter
    {
        Matrix4 viewMatrix;
        Matrix4 projectionMatrix;
        Matrix4 inverseProjectionMatrix;
        Vector2 screenDimensions;
        Vector4 cameraPosition;
    } viewParams;
    Gfx::PRenderPass renderPass;
    PRenderGraph renderGraph;
};
DEFINE_REF(RenderPass)
} // namespace Seele
