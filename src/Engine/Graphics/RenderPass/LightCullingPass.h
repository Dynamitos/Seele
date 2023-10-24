#pragma once
#include "RenderPass.h"
#include "Graphics/GraphicsResources.h"
#include "Scene/Scene.h"

namespace Seele
{
DECLARE_REF(CameraActor)
DECLARE_REF(Scene)
DECLARE_REF(Viewport)
struct LightCullingPassData
{
    LightEnv lightEnv;
};
class LightCullingPass : public RenderPass
{
public:
    LightCullingPass(Gfx::PGraphics graphics);
    virtual ~LightCullingPass();
    virtual void beginFrame(const Component::Camera& cam) override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    void setupFrustums();
    static constexpr uint32 BLOCK_SIZE = 32;
    static constexpr uint32 INDEX_LIGHT_ENV = 1;
    struct DispatchParams
    {
        glm::uvec3 numThreadGroups;
        uint32_t pad0;
        glm::uvec3 numThreads;
        uint32_t pad1;
    } dispatchParams;
    struct Plane
    {
        Vector n;
        float d;
    };
    struct Frustum
    {
        Plane planes[4];
    };
        
    Gfx::PShaderBuffer frustumBuffer;
    Gfx::PUniformBuffer dispatchParamsBuffer;
    Gfx::PUniformBuffer viewParamsBuffer;
    Gfx::PDescriptorSet frustumDescriptorSet;
    Gfx::PComputeShader frustumShader;
    Gfx::PPipelineLayout frustumLayout;
    Gfx::PComputePipeline frustumPipeline;

    Gfx::PTexture2D depthAttachment;
    Gfx::PShaderBuffer frustums;
    Gfx::PShaderBuffer oLightIndexCounter;
    Gfx::PShaderBuffer tLightIndexCounter;
    Gfx::PShaderBuffer oLightIndexList;
    Gfx::PShaderBuffer tLightIndexList;
    Gfx::PTexture2D oLightGrid;
    Gfx::PTexture2D tLightGrid;
    Gfx::PDescriptorSet lightEnvDescriptorSet;
    Gfx::PDescriptorSet cullingDescriptorSet;
    Gfx::PDescriptorLayout lightEnvDescriptorLayout;
    Gfx::PDescriptorLayout cullingDescriptorLayout;
    Gfx::PComputeShader cullingShader;
    Gfx::PPipelineLayout cullingLayout;
    Gfx::PComputePipeline cullingPipeline;
};
DEFINE_REF(LightCullingPass)
} // namespace Seele
