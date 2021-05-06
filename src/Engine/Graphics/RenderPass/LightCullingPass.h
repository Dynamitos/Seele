#pragma once
#include "RenderPass.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(CameraComponent)
DECLARE_NAME_REF(Gfx, Viewport)
DECLARE_NAME_REF(Gfx, Graphics)
class LightCullingPass : public RenderPass
{
public:
    LightCullingPass(PRenderGraph renderGraph, Gfx::PViewport viewport, Gfx::PGraphics graphics, PCameraActor camera);
    virtual ~LightCullingPass();
    virtual void beginFrame() override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    static constexpr uint32 BLOCK_SIZE = 8;
    _declspec(align(16)) struct DispatchParams
    {
        glm::uvec3 numThreadGroups;
        uint32_t pad0;
        glm::uvec3 numThreads;
        uint32_t pad1;
    } dispatchParams;
    __declspec(align(16)) struct Plane
    {
        Vector n;
        float d;
        Vector p0;
        Vector p1;
        Vector p2;
    };
    __declspec(align(16)) struct Frustum
    {
        Plane planes[4];
    };
    struct ScreenToView
    {
        Matrix4 inverseProjection;
        Vector2 screenDimensions;
    };
    Gfx::PViewport viewport;
    Gfx::PGraphics graphics;
    PCameraComponent source;
};
} // namespace Seele
