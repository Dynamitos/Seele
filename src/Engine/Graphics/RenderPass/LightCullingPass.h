#pragma once
#include "RenderPass.h"
#include "Graphics/GraphicsResources.h"
#include "Scene/Scene.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(CameraComponent)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
struct LightCullingPassData
{
    LightEnv lightEnv;
};
class LightCullingPass : public RenderPass<LightCullingPassData>
{
public:
    LightCullingPass(Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor camera);
    virtual ~LightCullingPass();
    virtual void beginFrame() override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    void setupFrustums();
    static constexpr uint32 BLOCK_SIZE = 8;
    static constexpr uint32 INDEX_LIGHT_ENV = 1;
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
        
    Gfx::PStructuredBuffer frustumBuffer;
    Gfx::PUniformBuffer dispatchParamsBuffer;
    Gfx::PUniformBuffer viewParamsBuffer;
    Gfx::PDescriptorSet frustumDescriptorSet;
    Gfx::PComputeShader frustumShader;
    Gfx::PPipelineLayout frustumLayout;
    Gfx::PComputePipeline frustumPipeline;

    Gfx::PTexture2D depthAttachment;
    Gfx::PStructuredBuffer frustums;
    Gfx::PStructuredBuffer oLightIndexCounter;
    Gfx::PStructuredBuffer tLightIndexCounter;
    Gfx::PStructuredBuffer oLightIndexList;
    Gfx::PStructuredBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;
    Gfx::PStructuredBuffer directLightBuffer;
    Gfx::PUniformBuffer numDirLightBuffer;
    Gfx::PStructuredBuffer pointLightBuffer;
    Gfx::PUniformBuffer numPointLightBuffer;
    Gfx::PDescriptorSet lightEnvDescriptorSet;
    Gfx::PDescriptorSet cullingDescriptorSet;
    Gfx::PDescriptorLayout lightEnvDescriptorLayout;
    Gfx::PDescriptorLayout cullingDescriptorLayout;
    Gfx::PComputeShader cullingShader;
    Gfx::PPipelineLayout cullingLayout;
    Gfx::PComputePipeline cullingPipeline;
    PCameraComponent source;
};
DEFINE_REF(LightCullingPass)
} // namespace Seele
