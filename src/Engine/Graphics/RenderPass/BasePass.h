#pragma once
#include "MinimalEngine.h"
#include "MeshProcessor.h"
#include "RenderPass.h"

namespace Seele
{
class BasePassMeshProcessor : public MeshProcessor
{
public:
    BasePassMeshProcessor(Gfx::PViewport viewport, Gfx::PGraphics graphics, uint8 translucentBasePass);
    virtual ~BasePassMeshProcessor();

    virtual void addMeshBatch(
        const MeshBatch& batch, 
//        const PPrimitiveComponent primitiveComponent,
        const Gfx::PRenderPass renderPass,
        Gfx::PPipelineLayout pipelineLayout,
        Gfx::PDescriptorLayout primitiveLayout,
        Array<Gfx::PDescriptorSet>& descriptorSets,
        int32 staticMeshId = -1) override;
    Array<Gfx::PRenderCommand> getRenderCommands();
    void clearCommands();
private:
    Array<Gfx::PRenderCommand> renderCommands;
    //Array<Gfx::PDescriptorSet> cachedPrimitiveSets;
    Gfx::PViewport target;
    uint8 translucentBasePass;
    //uint32 cachedPrimitiveIndex;
};
DEFINE_REF(BasePassMeshProcessor)
DECLARE_REF(CameraActor)
DECLARE_REF(CameraComponent)
struct BasePassData
{
    Array<StaticMeshBatch> staticDrawList;
};
class BasePass : public RenderPass<BasePassData>
{
public:
    BasePass(Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source);
    virtual ~BasePass();
    virtual void beginFrame() override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    Gfx::PRenderTargetAttachment colorAttachment;
    Gfx::PTexture2D depthBuffer;
    UPBasePassMeshProcessor processor;
    
    Array<Gfx::PDescriptorSet> descriptorSets;
    PCameraComponent source;
    Gfx::PPipelineLayout basePassLayout;
    // Set 0: Light environment
    static constexpr uint32 INDEX_LIGHT_ENV = 0;
    Gfx::PStructuredBuffer directLightBuffer;
    Gfx::PUniformBuffer numDirLightBuffer;
    Gfx::PStructuredBuffer pointLightBuffer;
    Gfx::PUniformBuffer numPointLightBuffer;
    Gfx::PStructuredBuffer oLightIndexList;
    Gfx::PTexture oLightGrid;
    Gfx::PDescriptorLayout lightLayout;
    // Set 1: viewParameter
    static constexpr uint32 INDEX_VIEW_PARAMS = 1;
    Gfx::PDescriptorLayout viewLayout;
    Gfx::PUniformBuffer viewParamBuffer;
    // Set 2: materials, generated
    static constexpr uint32 INDEX_MATERIAL = 2;
    // Set 3: primitive scene data
    static constexpr uint32 INDEX_SCENE_DATA = 3;
    Gfx::PDescriptorLayout primitiveLayout;
    Gfx::PUniformBuffer primitiveUniformBuffer;
    friend class BasePassMeshProcessor;
};
DEFINE_REF(BasePass)
} // namespace Seele
