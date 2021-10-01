#pragma once
#include "MinimalEngine.h"
#include "MeshProcessor.h"
#include "RenderPass.h"

namespace Seele
{
class DepthPrepassMeshProcessor : public MeshProcessor
{
public:
    DepthPrepassMeshProcessor(Gfx::PViewport viewport, Gfx::PGraphics graphics);
    virtual ~DepthPrepassMeshProcessor();
    
    virtual void addMeshBatch(
        const MeshBatch& batch,
        const Gfx::PRenderPass renderPass,
        Gfx::PPipelineLayout pipelineLayout,
        Gfx::PDescriptorLayout primitiveLayout,
        Array<Gfx::PDescriptorSet>& descriptorSets,
        int32 staticMeshId = -1) override;

    Array<Gfx::PRenderCommand> getRenderCommands();
    void clearCommands();
private:
    Array<Gfx::PRenderCommand> renderCommands;
    Array<Gfx::PDescriptorSet> cachedPrimitiveSets;
    Gfx::PViewport target;
    uint32 cachedPrimitiveIndex;
};
DEFINE_REF(DepthPrepassMeshProcessor)
DECLARE_REF(CameraActor)
DECLARE_REF(CameraComponent)
struct DepthPrepassData
{
    const Array<StaticMeshBatch> staticDrawList;
};
class DepthPrepass : public RenderPass<DepthPrepassData>
{
public:
    DepthPrepass(Gfx::PGraphics graphics, Gfx::PViewport viewport, PCameraActor source);
    ~DepthPrepass();
    virtual void beginFrame() override;
    virtual void render() override;
    virtual void endFrame() override;
    virtual void publishOutputs() override;
    virtual void createRenderPass() override;
    static void modifyRenderPassMacros(Map<const char*, const char*>& defines);
private:
    Gfx::PRenderTargetAttachment depthAttachment;
    Gfx::PTexture2D depthBuffer;
    UPDepthPrepassMeshProcessor processor;
    
    Array<Gfx::PDescriptorSet> descriptorSets;
    PCameraComponent source;
    Gfx::PPipelineLayout depthPrepassLayout;
    // Set 0: viewParameter
    static constexpr uint32 INDEX_VIEW_PARAMS = 0;
    Gfx::PDescriptorLayout viewLayout;
    Gfx::PUniformBuffer viewParamBuffer;
    // Set 1: materials, generated
    static constexpr uint32 INDEX_MATERIAL = 1;
    // Set 2: primitive scene data
    static constexpr uint32 INDEX_SCENE_DATA = 2;
    Gfx::PDescriptorLayout primitiveLayout;
    Gfx::PUniformBuffer primitiveUniformBuffer;
    friend class DepthPrepassMeshProcessor;
};
DEFINE_REF(DepthPrepass)
} // namespace Seele
