#pragma once
#include "RenderPass.h"

namespace Seele
{
class RayTracingPass : public RenderPass
{
public:
    RayTracingPass(Gfx::PGraphics graphics, PScene scene);
    RayTracingPass(RayTracingPass&& other) = default;
    RayTracingPass& operator=(RayTracingPass&& other) = default;
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
private:
    
};
}